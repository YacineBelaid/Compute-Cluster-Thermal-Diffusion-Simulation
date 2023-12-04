//
// Simulation de diffusion, basé sur l'exemple ex1p de MFEM.
//
// Description:  This example code demonstrates the use of MFEM to define a
//               simple finite element discretization of the Laplace problem
//               -Delta u = 1 with homogeneous Dirichlet boundary conditions.
//               Specifically, we discretize using a FE space of the specified
//               order, or if order < 1 using an isoparametric/isogeometric
//               space (i.e. quadratic for quadratic curvilinear mesh, NURBS for
//               NURBS mesh, etc.)
//
//               The example highlights the use of mesh refinement, finite
//               element grid functions, as well as linear and bilinear forms
//               corresponding to the left-hand side and right-hand side of the
//               discrete linear system. We also cover the explicit elimination
//               of essential boundary conditions, static condensation, and the
//               optional connection to the GLVis tool for visualization.

#include "mfem.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <string>
using namespace std;
using namespace mfem;

int main(int argc, char *argv[]) {

 // 1. Initialize MPI and HYPRE.
  auto start_program = std::chrono::steady_clock::now();
  Mpi::Init();
//  int num_procs = Mpi::WorldSize();
  int myid = Mpi::WorldRank();
  Hypre::Init();

  // 2. Parse command-line options.
  const char *mesh_file = "../data/square-disc.mesh";
  int order = 1;
  int par_ref_levels = 1;
  const char *output = "Heatsim";
  int processors = 1; //par défaut

  OptionsParser args(argc, argv);
  args.AddOption(&mesh_file, "-m", "--mesh", "Mesh file to use.");
  args.AddOption(&order, "-o", "--order",
                 "Finite element order (polynomial degree) or -1 for"
                 " isoparametric space.");
  args.AddOption(&par_ref_levels, "-rp", "--refine-parallel",
                 "Number of times to refine the mesh uniformly in parallel.");
  args.AddOption(&output, "-n", "--name", "Output data collection name");
  args.AddOption(&processors, "-p", "--processeurs", "Number of processors used for output");

  args.Parse();
  if (!args.Good()) {
    if (myid == 0) {
      args.PrintUsage(cout);
    }
    return 1;
  }
  if (myid == 0) {
    args.PrintOptions(cout);
  }

  // CHARGEMENT ET RAFFINEMENT DU MAILLAGE
  // 4. Read the (serial) mesh from the given mesh file on all processors.  We
  //    can handle triangular, quadrilateral, tetrahedral, hexahedral, surface
  //    and volume meshes with the same code.
  auto start_mesh = std::chrono::steady_clock::now();
  Mesh mesh(mesh_file, 1, 1);
  int dim = mesh.Dimension();
  if (myid == 0) {
    std::cout << "Number of Elements before refinement: " << mesh.GetNE()
              << std::endl;
  }

  // 6. Define a parallel mesh by a partitioning of the serial mesh. Refine
  //    this mesh further in parallel to increase the resolution. Once the
  //    parallel mesh is defined, the serial mesh can be deleted.
  ParMesh pmesh(MPI_COMM_WORLD, mesh);
  mesh.Clear();
  for (int l = 0; l < par_ref_levels; l++) {
    pmesh.UniformRefinement();
  }

  if (myid == 0) {
    std::cout << "Number of Elements after refinement: " << pmesh.GetNE()
              << std::endl;
  }
  auto end_mesh = std::chrono::steady_clock::now();
  double duration_mesh = std::chrono::duration_cast<std::chrono::milliseconds>(end_mesh - start_mesh).count();

  // ASSEMBLAGE
  // 7. Define a parallel finite element space on the parallel mesh. Here we
  //    use continuous Lagrange finite elements of the specified order. If
  //    order < 1, we instead use an isoparametric/isogeometric space.
  auto start_assembly = std::chrono::steady_clock::now();
  FiniteElementCollection *fec;
  bool delete_fec;
  if (order > 0) {
    fec = new H1_FECollection(order, dim);
    delete_fec = true;
  } else if (pmesh.GetNodes()) {
    fec = pmesh.GetNodes()->OwnFEC();
    delete_fec = false;
    if (myid == 0) {
      cout << "Using isoparametric FEs: " << fec->Name() << endl;
    }
  } else {
    fec = new H1_FECollection(order = 1, dim);
    delete_fec = true;
  }
  ParFiniteElementSpace fespace(&pmesh, fec);
  HYPRE_BigInt size = fespace.GlobalTrueVSize();
  if (myid == 0) {
    cout << "Number of finite element unknowns: " << size << endl;
  }

  // 8. Determine the list of true (i.e. parallel conforming) essential
  //    boundary dofs. In this example, the boundary conditions are defined
  //    by marking all the boundary attributes from the mesh as essential
  //    (Dirichlet) and converting them to a list of true dofs.
  Array<int> ess_tdof_list;
  if (pmesh.bdr_attributes.Size()) {
    Array<int> ess_bdr(pmesh.bdr_attributes.Max());
    ess_bdr = 1;
    fespace.GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
  }
  auto end_assembly = std::chrono::steady_clock::now();
  double duration_assembly = std::chrono::duration_cast<std::chrono::milliseconds>(end_assembly - start_assembly).count();

  // RÉSOLUTION
  // 9. Set up the parallel linear form b(.) which corresponds to the
  //    right-hand side of the FEM linear system, which in this case is
  //    (1,phi_i) where phi_i are the basis functions in fespace.
  auto start_resolution = std::chrono::steady_clock::now();
  ParLinearForm b(&fespace);
  ConstantCoefficient one(1.0);
  b.AddDomainIntegrator(new DomainLFIntegrator(one));
  b.Assemble();

  // 10. Define the solution vector x as a parallel finite element grid
  //     function corresponding to fespace. Initialize x with initial guess of
  //     zero, which satisfies the boundary conditions.
  ParGridFunction x(&fespace);
  x = 0.0;

  // 11. Set up the parallel bilinear form a(.,.) on the finite element space
  //     corresponding to the Laplacian operator -Delta, by adding the
  //     Diffusion domain integrator.
  ParBilinearForm a(&fespace);
  a.AddDomainIntegrator(new DiffusionIntegrator(one));
  a.Assemble();

  OperatorPtr A;
  Vector B, X;
  a.FormLinearSystem(ess_tdof_list, x, b, A, X, B);

  // 13. Solve the linear system A X = B.
  //     * With full assembly, use the BoomerAMG preconditioner from hypre.
  //     * With partial assembly, use Jacobi smoothing, for now.
  Solver *prec = new HypreBoomerAMG;

  CGSolver cg(MPI_COMM_WORLD);
  cg.SetRelTol(1e-12);
  cg.SetMaxIter(2000);
  cg.SetPrintLevel(1);
  if (prec) {
    cg.SetPreconditioner(*prec);
  }
  cg.SetOperator(*A);
  cg.Mult(B, X);
  delete prec;

  // 14. Recover the parallel grid function corresponding to X. This is the
  //     local finite element solution on each processor.
  a.RecoverFEMSolution(X, b, x);
  auto end_resolution = std::chrono::steady_clock::now();
  double duration_resolution = std::chrono::duration_cast<std::chrono::milliseconds>(end_resolution - start_resolution).count();

  // SAUVEGARDE DES RÉSULTATS
  // 15. Save the refined mesh and the solution in parallel.
  auto start_save = std::chrono::steady_clock::now();
  {
    ParGridFunction partition(&fespace);
    partition = static_cast<double>(myid);

    ParaViewDataCollection *pd = NULL;
    pd = new ParaViewDataCollection(output, &pmesh);
    pd->SetPrefixPath("ParaView");
    pd->RegisterField("solution", &x);
    pd->RegisterField("partition", &partition);
    pd->SetLevelsOfDetail(order);
    pd->SetDataFormat(VTKFormat::BINARY);
    pd->SetHighOrderOutput(true);
    pd->SetCycle(0);
    pd->SetTime(0.0);
    pd->Save();
    delete pd;
  }
  auto end_save = std::chrono::steady_clock::now();
  double duration_save = std::chrono::duration_cast<std::chrono::milliseconds>(end_save - start_save).count();

  // 17. Free the used memory.
  if (delete_fec) {
    delete fec;
  }

  auto end_program = std::chrono::steady_clock::now();
  double program_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_program - start_program).count();
  double dofs_per_second = size / (program_duration / 1000.0);

  //ECRITURE
 
  std::string output_name = "Heatsim_ p_"+ std::to_string(processors)+"rp_"+std::to_string(par_ref_levels);
  std::ofstream outputFile(output_name + "_timeMeasurement.csv");
  if (outputFile.is_open()) {
        outputFile << "nombre de processeurs : " << processors << std::endl;
        outputFile << "nombre de raffinement parrallel :" << par_ref_levels << std::endl;
        outputFile << "Resultats : " <<  std::endl;
        outputFile << "Raffinement : " << duration_mesh << "milliseconds." << std::endl;
        outputFile << "Assemblage : " << duration_assembly << "milliseconds." << std::endl;
        outputFile << "Résolution : " << duration_resolution << "milliseconds." << std::endl;
        outputFile << "Sauvegarde : " << duration_save << "milliseconds." << std::endl;
        outputFile << "Degrees of Freedom : " << size << std::endl;
        outputFile << "Duree du programme : " << program_duration << " milliseconds" << endl;
        outputFile << "DoF/s: " << dofs_per_second << endl;
        outputFile.close();
  } else {
      std::cerr << "Impossible d'ouvrir le fichier CPU_timeMeasurement.csv pour écriture." << std::endl;
      return -1;
  }

  return 0;
}

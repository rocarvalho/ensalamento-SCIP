#include <sstream>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <random>
#include <unistd.h> // *POSIX* Para o getopt() original
#include <getopt.h> // *GNU* Para o getopt_long()
#include <string>
#include <map>
#include <vector>
#include <cfloat>
#include <limits>
#include <objscip/objscip.h>
#include <objscip/objscipdefplugins.h>
#include <fstream>

using namespace std;

    typedef vector<SCIP_VAR *> v1d;
    typedef vector<v1d> v2d;
    typedef vector<v2d> v3d;
    typedef vector<SCIP_SOL *> v1da;
    typedef vector<v1da> v2da;
    typedef vector<v2da> v3da;


struct info_turma{
    int pos;
    char nome[15];
};

struct info_sala{
    int numero;
    int capacidade;
    int ar_condicionado;
};


int main(int argc, char** argv) {

    // Obtain a seed from the system clock to initialize the random number generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    // Initialize the random number generator based on Mersenne Twister engine
    std::mt19937 generator(seed);

    char dados_horario[30] = ""; //guardar caminho da instancia
    char dados_sala[30] = ""; //guardar caminho da instancia
    double alpha1 = 100;
    double alpha2 = 100;
    double alpha3 = 100;
    
    //opcoes de entrada terminal
    struct option OpcoesLongas[] = {
            {"ajuda", no_argument, NULL, 'a'},            
            {"arquivo horario", required_argument, NULL, 'h'},
            {"arquivo sala", required_argument, NULL, 's'},
            {0, 0, 0, 0}
    };    

    if(argc == 1) { // Sem argumentos
                printf("Parametros faltando\n");
                exit(0);
    }

    
    char optc = 0;  // Parece estranho... Mas todo CHAR é na verdade um INT
    while((optc = getopt_long(argc, argv, "h:a:s:", OpcoesLongas, NULL)) != -1) {
            switch(optc) {
                case 'a' : 
                    break;                                   
                case 'h' :
                    strcpy(dados_horario, optarg);
                    break;
                case 's' :
                    strcpy(dados_sala, optarg);
                    break;
                    
                default : // Qualquer parametro nao tratado
                            printf("Parametros incorretos.\n");
                            exit(0);
            }
    }                  

    ifstream in;
    in.open(dados_horario);
    
    map<string, int> turmas;        
    int num_dias, num_horarios, num_salas, num_turmas;
    
    if (!in.is_open()){
       cout << "Error openning file: " << dados_horario << endl;
       in.close();
       system("pause");
       exit(EXIT_FAILURE);
    }

    
    in >> num_horarios;     
    in >> num_salas;
    in >> num_turmas;         
    
    int **horario;
    horario = (int**)malloc(num_turmas*sizeof(int*));   
    for(int i = 0; i < num_turmas; i++){
        horario[i] = (int*)malloc(num_horarios*sizeof(int));
    }    
    
    int *demanda = new int[num_turmas];    
    
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            horario[i][j] = 0;            
        }
        demanda[i] = 0;
    }
    
    vector<info_sala> salas;
    
    for(int i = 0; i < num_salas; i++){
        info_sala sala;
        in >> sala.numero;
        in >> sala.capacidade;
        in >> sala.ar_condicionado;
        salas.push_back(sala);        
    }
       
    string aux;    
    for(int i = 0; i < num_turmas; i++){
        in >> aux;         
        turmas[aux] = i;
        in >> demanda[i]; 
    }
    
    for(map<string, int>::iterator pos = turmas.begin(); pos != turmas.end(); pos++){
        cout << pos->first << "\t" << pos->second << endl;
    }
    
    string t;
    int h;
    map<string, int>::iterator pos;
    while(!in.eof()){                
        
        in >> t;
        in >> h;       
                
        if(!t.empty()){
            cout << " * " << t << " ";
            pos = turmas.find(t);     
            cout << pos->second << endl;
            horario[pos->second][h] = 1;                
        }
    }
        
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            cout << horario[i][j] << " ";
        }
        cout << endl;
    }   



    // Create an instance of SCIP engine
    SCIP* scip = NULL;
    SCIP_CALL(SCIPcreate(&scip));

    // Include SCIP's default plugins
    SCIP_CALL(SCIPincludeDefaultPlugins(scip));

    // Initialize SCIP to write the MIP model
    SCIP_CALL(SCIPcreateProbBasic(scip, "Assignment"));

    // Objective sense
    SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));

    // Variables 	
    v3d r(num_turmas, v2d(num_horarios, v1d(num_salas, NULL)));

    ostringstream namebuf;
    
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            for(int k = 0; k < num_salas; k++){
                namebuf.str("");
                namebuf << "r#" << i << "#" << j << "#" << k;
		SCIP_VAR* var;
		SCIP_CALL(SCIPcreateVarBasic(scip,
		        &var,                  // Pointer to variable handler
		        namebuf.str().c_str(),                  // String with a name to variable
		        0.0,                   // Lower bound
		        DBL_MAX,                   // Upper bound
		        100,             // Objective coefficient
		        SCIP_VARTYPE_CONTINUOUS)); // Type of variable

		SCIP_CALL(SCIPaddVar(scip, var));
		r[i][j][k] = var;
                  
            }
        }
    }

//    vector< vector< vector <SCIP_VAR *> > > s_p;
    v3d s_p(num_turmas, v2d(num_horarios, v1d(num_salas, NULL)));
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            for(int k = 0; k < num_salas; k++){
		SCIP_VAR* var;
		SCIP_CALL(SCIPcreateVarBasic(scip,
		        &var,                  // Pointer to variable handler
		        NULL,                  // String with a name to variable
		        0.0,                   // Lower bound
		        DBL_MAX,                   // Upper bound
		        100,             // Objective coefficient
		        SCIP_VARTYPE_CONTINUOUS)); // Type of variable

		SCIP_CALL(SCIPaddVar(scip, var));
		s_p[i][j][k] = var;
                  
            }
        }
    }

    //vector< vector< vector <SCIP_VAR *> > > s_n;
    v3d s_n(num_turmas, v2d(num_horarios, v1d(num_salas, NULL)));
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            for(int k = 0; k < num_salas; k++){
		SCIP_VAR* var;
		SCIP_CALL(SCIPcreateVarBasic(scip,
		        &var,                  // Pointer to variable handler
		        NULL,                  // String with a name to variable
		        0.0,                   // Lower bound
		        DBL_MAX,                   // Upper bound
		        100,             // Objective coefficient
		        SCIP_VARTYPE_CONTINUOUS)); // Type of variable

		SCIP_CALL(SCIPaddVar(scip, var));
		s_n[i][j][k] = var;
                  
            }
        }
    }

    //vector< vector< vector <SCIP_VAR *> > > xx;
    v3d xx(num_turmas, v2d(num_horarios, v1d(num_salas, NULL)));
    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
            for(int k = 0; k < num_salas; k++){
		SCIP_VAR* var;
		SCIP_CALL(SCIPcreateVarBasic(scip,
		        &var,                  // Pointer to variable handler
		        NULL,                  // String with a name to variable
		        0.0,                   // Lower bound
		        1.0,                   // Upper bound
		        0,             // Objective coefficient
		        SCIP_VARTYPE_BINARY)); // Type of variable

		SCIP_CALL(SCIPaddVar(scip, var));
		xx[i][j][k] = var;
                  
            }
        }
    }

    SCIP_VAR* tt[num_turmas];
    for (int i = 0; i < num_turmas; ++i) {
        SCIP_VAR* var;
        SCIP_CALL(SCIPcreateVarBasic(scip,
                &var,                  // Pointer to variable handler
                NULL,                  // String with a name to variable
                0.0,                   // Lower bound
                DBL_MAX,                   // Upper bound
                100,             // Objective coefficient
                SCIP_VARTYPE_CONTINUOUS)); // Type of variable

        SCIP_CALL(SCIPaddVar(scip, var));
        tt[i] = var;
    }


    vector< vector <SCIP_VAR *> > v(num_turmas, vector <SCIP_VAR *>(num_salas));
    for (int i = 0; i < num_turmas; ++i) {
	for(int j = 0; j < num_salas; ++j){        
		SCIP_VAR* var;
		SCIP_CALL(SCIPcreateVarBasic(scip,
		        &var,                  // Pointer to variable handler
		        NULL,                  // String with a name to variable
		        0.0,                   // Lower bound
		        DBL_MAX,                   // Upper bound
		        100,             // Objective coefficient
		        SCIP_VARTYPE_CONTINUOUS)); // Type of variable

		SCIP_CALL(SCIPaddVar(scip, var));
		v[i][j] = var;
        }
    }



    // Constraint

    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
	    for(int k = 0; k < num_salas; k++){
		    SCIP_CONS* constraint;
		    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			    &constraint,           // Pointer to constraint handler
			    "",                    // String with name to constraint
			    0, NULL, NULL,         // Number of variables, variables, coefficients
			    -SCIPinfinity(scip),   // LHS
			    SCIPinfinity(scip)));  // RHS
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				xx[i][j][k],         // Variable handler
				demanda[i]-(salas[k].ar_condicionado*demanda[i])));  // Coefficient
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				r[i][j][k],         // Variable handler
				-1));  // Coefficient
			    SCIP_CALL(SCIPchgRhsLinear(scip,
				constraint,   // Constraint handler
				60));   // RHS	  
			    SCIP_CALL(SCIPaddCons(scip, constraint));  		
	    }
    	}
    } 

    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
	    for(int k = 0; k < num_salas; k++){
		    SCIP_CONS* constraint;
		    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			    &constraint,           // Pointer to constraint handler
			    "",                    // String with name to constraint
			    0, NULL, NULL,         // Number of variables, variables, coefficients
			    0,   // LHS
			    0));  // RHS
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				xx[i][j][k],         // Variable handler
				(salas[k].capacidade - demanda[i])));  // Coefficient
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				s_p[i][j][k],         // Variable handler
				1));  // Coefficient
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				s_n[i][j][k],         // Variable handler
				-1));
			    SCIP_CALL(SCIPaddCons(scip, constraint));			    
	    }
    	}
    } 


    for(int j = 0; j < num_horarios; j++){
       for(int k = 0; k < num_salas; k++){
	    SCIP_CONS* uma_turma_sala_por_horario;
	    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
		    &uma_turma_sala_por_horario,           // Pointer to constraint handler
		    "",                    // String with name to constraint
		    0, NULL, NULL,         // Number of variables, variables, coefficients
		    -SCIPinfinity(scip),   // LHS
		    SCIPinfinity(scip)));  // RHS
       
           for(int i = 0; i < num_turmas; i++){
		SCIP_CALL(SCIPaddCoefLinear(scip,
		        uma_turma_sala_por_horario,   // Constraint handler
		        xx[i][j][k],         // Variable handler
		        1));  // Coefficient
           }


	    SCIP_CALL(SCIPchgRhsLinear(scip,
		    uma_turma_sala_por_horario,   // Constraint handler
		    1));   // RHS

	    SCIP_CALL(SCIPaddCons(scip, uma_turma_sala_por_horario));
       }
    }

    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
	    for(int k = 0; k < num_salas; k++){
		    SCIP_CONS* constraint;
		    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			    &constraint,           // Pointer to constraint handler
			    "",                    // String with name to constraint
			    0, NULL, NULL,         // Number of variables, variables, coefficients
			    -SCIPinfinity(scip),   // LHS
			    SCIPinfinity(scip)));  // RHS
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				xx[i][j][k],         // Variable handler
				1));  // Coefficient
			    SCIP_CALL(SCIPaddCoefLinear(scip,
				constraint,   // Constraint handler
				v[i][k],         // Variable handler
				-1));  // Coefficient			   
			    SCIP_CALL(SCIPchgRhsLinear(scip,
				    constraint,   // Constraint handler
				    0));   // RHS
			    SCIP_CALL(SCIPaddCons(scip, constraint));

	    }
    	}
    } 

    for(int i = 0; i < num_turmas; i++){
	SCIP_CONS* constraint;
	    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
	    &constraint,           // Pointer to constraint handler
	    "",                    // String with name to constraint
	    0, NULL, NULL,         // Number of variables, variables, coefficients
	    -SCIPinfinity(scip),   // LHS
	    SCIPinfinity(scip)));  // RHS
        for(int k = 0; k < num_salas; k++){
	    SCIP_CALL(SCIPaddCoefLinear(scip,
		constraint,   // Constraint handler
		v[i][k],         // Variable handler
		1));  // Coefficient    
	}
        SCIP_CALL(SCIPaddCoefLinear(scip,
	  constraint,   // Constraint handler
	  tt[i],         // Variable handler
	  -1));  // Coefficient         
    	SCIP_CALL(SCIPchgRhsLinear(scip,
	  constraint,   // Constraint handler
	  1));   // RHS	
        SCIP_CALL(SCIPaddCons(scip, constraint));			
    }

    for(int i = 0; i < num_turmas; i++){
        for(int j = 0; j < num_horarios; j++){
	    SCIP_CONS* constraint;
		    SCIP_CALL(SCIPcreateConsBasicLinear(scip,
		    &constraint,           // Pointer to constraint handler
		    "",                    // String with name to constraint
		    0, NULL, NULL,         // Number of variables, variables, coefficients
		    horario[i][j],   // LHS
		    horario[i][j]));  // RHS

	    for(int k = 0; k < num_salas; k++){
		    SCIP_CALL(SCIPaddCoefLinear(scip,
			constraint,   // Constraint handler
			xx[i][j][k],         // Variable handler
			1));  // Coefficient  
	    }
            SCIP_CALL(SCIPaddCons(scip, constraint));
        }
     }

    // Solve model
    SCIP_CALL(SCIPsolve(scip));

    // Get best solution found
    SCIP_SOL * sol = SCIPgetBestSol(scip);


	for(int i = 0; i < num_turmas; i++){
	    for(int j = 0; j < num_horarios; j++){
		bool print = true;
		for(int k = 0; k < num_salas; k++){
		    if(SCIPgetSolVal(scip, sol, xx[i][j][k]) > 0.5){
		        cout << salas[k].numero << "\t";
		        print = false;
		    }                            
		}
		if (print) cout << "---\t";
	    }
	    cout << endl;
	}  
     
    std::cout << "Objective function: " << SCIPgetSolOrigObj(scip, sol) << std::endl;

    //SCIP_CALL( SCIPprintBestSol(scip, NULL, true) );

    /* Print objective value and solution found
    std::cout << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Objective function: " << SCIPgetSolOrigObj(scip, solution) << std::endl;
    std::cout << std::endl;
    std::cout << "Solution: ";
    for (int i = 0; i < n; ++i) {
        if (x_values[i] > 0.5) {
            std::cout << i << " ";
        }
    }
    std::cout << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << std::endl;

    // Free resources
    SCIP_CALL(SCIPfreeSol(scip, &solution));
    SCIP_CALL(SCIPreleaseCons(scip, &constraint));
    for (int i = 0; i < n; ++i) {
        SCIP_CALL(SCIPreleaseVar(scip, &x[i]));
    }
    SCIP_CALL(SCIPfree(&scip));
    */

    return EXIT_SUCCESS;
}

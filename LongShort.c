#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

struct operativa 				// Estructura de datos para la información resultante de operar con futuros 
{
	char date_EU[11];
	char timetable[13];
	float max;
	float min;
	float range;
	float range_trail;
	float range_avg;
	float open;
	float longe_close;
	float shorte_close;
	float close;        // donde cierra el mercado a la hora_fin
	float longe_rst;
	float shorte_rst;
	float longe_acc;
	float shorte_acc;
	char hora_ini[6];
	char hora_fin[6];
    char operation[2];
    float target;
    float limit;
    float result_pts;
    float result_accum_pts;
    float contract_qty;
    float multiplier;
    float result_amt;
    float result_accum_amt;
    float longe_accum_tst;
    float shorte_accum_tst;
};

struct operativa datosdiarios[6500];  // son 15 años de datos de barras diarias. (5x52x25)

int leerFicheroOriginal(char mifichero[]);
int OperacionTecnicaLLEX(char ini_year[], char end_year[], char g_limit_qty[], char g_target_qty[], char g_multiplier[], char g_tic[]);
int OperacionTecnicaLSEX(char ini_year[], char end_year[], char g_limit_qty[], char g_target_qty[], char g_multiplier[], char g_tic[]);
int escribeResultadoTecnicaEX(char mifichero[]);
float ConvCadenaFloat(char cadena[]);
char *convEnteroCadena(int entero);
int convCadenaEntero(char cadena[]);
int posicionarseEnUnaFecha_EstructOperativa(char mifecha[]);
int calculaDiaSemana(char fecha[]);
char *MiDirectorioActual();
int ComprobarFicheroExiste(char mifichero[]);



int main (int argc, char *argv[])
{
	char MY_DATE[15], my_contract[5], my_year_ini[5], my_year_end[5], my_df[25], my_limit_qty[5], my_target_qty[5], my_multiplier[5], my_tic[5], my_sys[5], myd[250];
	char my_file[50], my_path[200], my_pathfile[250], a[]="4";
	
	int arg, seguir=0, b;

	for (arg=0; arg<argc; ++arg)
		printf("arg %d > %s || ", arg, argv[arg]);

	
	strcpy(myd,MiDirectorioActual());

	snprintf(my_contract, sizeof my_contract, "%s", argv[1]);

	snprintf(my_year_ini, sizeof my_year_ini, "%s", argv[2]);

	snprintf(my_year_end, sizeof my_year_end, "%s", argv[3]);

	snprintf(my_df, sizeof my_df, "%s", argv[4]);

	snprintf(my_limit_qty, sizeof my_limit_qty, "%s", argv[5]);

	snprintf(my_target_qty, sizeof my_target_qty, "%s", argv[6]);

	snprintf(my_multiplier, sizeof my_multiplier, "%s", argv[7]);

	snprintf(my_tic, sizeof my_tic, "%s", argv[8]);

	snprintf(my_sys, sizeof my_sys, "%s", argv[9]);

	printf("My_DF = %s\n", argv[4]);
	seguir=0;

	snprintf(my_file, sizeof my_file, "%s", argv[4]);
	snprintf(my_path, sizeof my_path, "%s","/home/agustin/ironhack/ih_datamadpt0420_project_final/Data/");
	snprintf(my_pathfile, sizeof my_pathfile, "%s%s", my_path, my_file);
	
	printf("%s\n", my_pathfile );

	

	seguir=leerFicheroOriginal(my_pathfile);                        //1

	if (seguir != 1)
	{
		printf("%s\n", "Error en la carga del fichero" );
		return 0;
	}

	if (strcmp(my_sys,"LLEX")==0)
	{
		seguir=seguir+OperacionTecnicaLLEX(my_year_ini, my_year_end, my_limit_qty, my_target_qty, my_multiplier, my_tic);   // 3
	}
	else if (strcmp(my_sys,"LSEX")==0)
	{
		seguir=seguir+OperacionTecnicaLSEX(my_year_ini, my_year_end, my_limit_qty, my_target_qty, my_multiplier, my_tic);      // 3
	}


	snprintf(my_pathfile, sizeof my_pathfile, "%s%s%s", my_path, my_contract, "_04.csv");
	printf("Generando output en %s\n", my_pathfile);
	seguir=seguir+escribeResultadoTecnicaEX(my_pathfile);                                  // 5

	printf("Finalizado ============ Output=%d\n",seguir);

	return seguir;

}


int OperacionTecnicaLLEX(char ini_year[], char end_year[], char g_limit_qty[], char g_target_qty[], char g_multiplier[], char g_tic[])

{
	char my_date_ini[10], my_date_end[10];
	int my_day, pt_ini, pt_end, cont, my_flag;

	my_flag=1;

	float limit_qty = strtof(g_limit_qty,NULL);               // from arguments?
	float target_qty = strtof(g_target_qty,NULL);               // from arguments?
	float tic_qty = strtof(g_tic,NULL);                   // from arguments?

// Definicion de variables a usar

	char longe[2]="L";
	char shorte[2]="S";
	char operation_ant[2]="L";
	float limit_ant= 0.0;
	float target_ant= 0.0;
	float limit_qty_rng= limit_qty;          // from arguments
	float target_qty_rng= target_qty;       // from arguments
	float axis_size = 0.0;
	float tic = tic_qty;                     // from arguments
	int contract_qty= 1;
	int multiplier= atoi(g_multiplier);

// Definicion de variables para la operación

	float longe_accum = 0.0;  
    float shorte_accum = 0.0;  
    float result_accum_amt = 0.0;  
    float result_accum_pts = 0.0;  
    float limit_nxt = 0 - limit_qty_rng * axis_size;  
    float target_nxt = 0 + target_qty_rng * axis_size;  
    char operation[2] = "L";
    char operation_nxt[2] = "L";
    int target_reached = 0;  
    float range_trail = 0.0;
    float longe_result = 0.0;
    float shorte_result = 0.0;
    float limit = 0.0;
    float target = 0.0;
    int continue_no_change = 0;


//  locate the initial position

    my_day = 2;
    snprintf(my_date_ini, sizeof my_date_ini, "%s%s%d", ini_year, "010", my_day);
 
 	while ((calculaDiaSemana(my_date_ini) == 0) || (calculaDiaSemana(my_date_ini) == 6))
 	{
 		my_day = my_day + 1;
 		snprintf(my_date_ini, sizeof my_date_ini, "%s%s%d", ini_year, "010", my_day);
 	}

 	printf("%s\n", my_date_ini);

 	pt_ini=posicionarseEnUnaFecha_EstructOperativa(my_date_ini);
 

//  locate the final position

	my_day = 31;
    snprintf(my_date_end, sizeof my_date_end, "%s%s%d", end_year, "12", my_day);
 
 	while ((calculaDiaSemana(my_date_ini) == 0) || (calculaDiaSemana(my_date_ini) == 6))
 	{
 		my_day = my_day - 1;
 		snprintf(my_date_end, sizeof my_date_end, "%s%s%d", end_year, "12", my_day);
 	}

 	printf("%s\n", my_date_end);

 	pt_end=posicionarseEnUnaFecha_EstructOperativa(my_date_end);

 	printf("%d - %d\n", pt_ini, pt_end );

 // the loop

 	cont=pt_ini;

 	while (cont <= pt_end)   // bucle
	{
		range_trail = datosdiarios[cont].range_trail;
		longe_result = datosdiarios[cont].longe_rst;
		shorte_result = datosdiarios[cont].shorte_rst;

		axis_size = range_trail;

		if (cont == pt_ini)
		{
			limit_nxt = 0 - limit_qty_rng * axis_size;
            target_nxt = 0 + target_qty_rng * axis_size;
		}

		strcpy(operation, operation_nxt);
		limit = limit_nxt;
		target = target_nxt;

		longe_accum = longe_accum + longe_result;
		shorte_accum = shorte_accum + shorte_result;

		continue_no_change=1;

		if (strcmp(operation, longe)==0)
		{
			result_accum_pts = result_accum_pts + longe_result;                                              // tic commision only for results in dollars not in pts
            result_accum_amt = result_accum_amt + (longe_result - tic) * contract_qty * multiplier;

            if ((longe_accum < limit) && (continue_no_change == 1))                            // overcome the limit and change to shorts
            {
            	strcpy(operation_ant, longe);
                strcpy(operation_nxt, shorte);
                limit_ant = limit;
                limit_nxt = longe_accum + (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum - (target_qty_rng * axis_size);
                continue_no_change = 0;
            }

            if ((target_reached == 1) && (longe_result <= ((0-1) * range_trail * 0.2)) && (continue_no_change == 1))        // target reached with reverse of 20%
            {
            	strcpy(operation_ant, longe);
                strcpy(operation_nxt, shorte);
                limit_ant = limit;
                limit_nxt = longe_accum + (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum - (target_qty_rng * axis_size);
                target_reached = 0;
                continue_no_change = 0;
            }

            if ((longe_accum > target) && (target_reached == 0) && (continue_no_change == 1))                        // target reached but waiting reverse)
            {
            	target_reached = 1;
                strcpy(operation_ant, operation);
                strcpy(operation_nxt, operation_ant);
                limit_ant = limit;
                limit_nxt = limit_ant;
                target_ant = target;
                target_nxt = target_ant;
                continue_no_change = 0;
            }

		}

		if (strcmp(operation, shorte)==0)
		{
			result_accum_pts = result_accum_pts + shorte_result;                                          // tic commision only for results in dollars not in pts
            result_accum_amt = result_accum_amt + (shorte_result - tic) * contract_qty * multiplier;

            if ((longe_accum > limit) && (continue_no_change == 1))                            // overcome the limit and change to long
            {
            	strcpy(operation_ant, shorte);
                strcpy(operation_nxt, longe);
                limit_ant = limit;
                limit_nxt = longe_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum + (target_qty_rng * axis_size);
                continue_no_change = 0;
            }

            if ((target_reached == 1) && (longe_result > (range_trail * 0.2)) && (continue_no_change == 1))        // target reached with reverse of 20%
            {
            	strcpy(operation_ant, shorte);
                strcpy(operation_nxt, longe);
                limit_ant = limit;
                limit_nxt = longe_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum + (target_qty_rng * axis_size);
                target_reached = 0;
                continue_no_change = 0;
            }

            if ((longe_accum < target) && (target_reached == 0) && (continue_no_change == 1))                        // target reached but waiting reverse)
            {
            	target_reached = 1;
                strcpy(operation_ant, operation);
                strcpy(operation_nxt, operation_ant);
                limit_ant = limit;
                limit_nxt = limit_ant;
                target_ant = target;
                target_nxt = target_ant;
                continue_no_change = 0;
            }
        }
        
        if (continue_no_change == 1)  // nothing happen, no target reached, no limit overcame, continue the same
        {
            strcpy(operation_ant, operation);
         	strcpy(operation_nxt, operation_ant);
          	limit_ant = limit;
           	limit_nxt = limit_ant;
           	target_ant = target;
           	target_nxt = target_ant;
        }


 // # save the daily operation

        strcpy(datosdiarios[cont].operation, operation_ant);
        datosdiarios[cont].target = target_ant;
        datosdiarios[cont].limit = limit_ant;
        datosdiarios[cont].result_pts = result_accum_pts - datosdiarios[cont-1].result_accum_pts;
        datosdiarios[cont].result_accum_pts = result_accum_pts;
        datosdiarios[cont].contract_qty = contract_qty;
        datosdiarios[cont].multiplier = multiplier;
        datosdiarios[cont].result_amt = result_accum_amt - datosdiarios[cont-1].result_accum_amt;
        datosdiarios[cont].result_accum_amt = result_accum_amt;
        datosdiarios[cont].longe_accum_tst = longe_accum;
        datosdiarios[cont].shorte_accum_tst = shorte_accum;

    	
        //printf("-->>%d\n", cont);
    	//printf("%s=%s=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f\n",datosdiarios[cont].date_EU,datosdiarios[cont].timetable,
		//			datosdiarios[cont].max,datosdiarios[cont].min,datosdiarios[cont].range,datosdiarios[cont].range_trail,
		//			datosdiarios[cont].range_avg,datosdiarios[cont].open,datosdiarios[cont].longe_close,datosdiarios[cont].shorte_close,
		//			datosdiarios[cont].close,datosdiarios[cont].longe_rst,datosdiarios[cont].shorte_rst,datosdiarios[cont].longe_acc,
		//			datosdiarios[cont].shorte_acc);
    	//printf("%s=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f\n",datosdiarios[cont].operation, datosdiarios[cont].target,datosdiarios[cont].limit,
    	//	datosdiarios[cont].result_pts, datosdiarios[cont].result_accum_pts, datosdiarios[cont].contract_qty, datosdiarios[cont].multiplier,
    	//	datosdiarios[cont].result_amt, datosdiarios[cont].result_accum_amt, datosdiarios[cont].longe_accum_tst, datosdiarios[cont].shorte_accum_tst);

    	cont = cont + 1;

    }

    return 3;

// Fin del código
}


int OperacionTecnicaLSEX(char ini_year[], char end_year[], char g_limit_qty[], char g_target_qty[], char g_multiplier[], char g_tic[])

{
	char my_date_ini[10], my_date_end[10];
	int my_day, pt_ini, pt_end, cont, my_flag;

	my_flag=1;

	float limit_qty = strtof(g_limit_qty,NULL);               // from arguments?
	float target_qty = strtof(g_target_qty,NULL);               // from arguments?
	float tic_qty = strtof(g_tic,NULL);                   // from arguments?

// Definicion de variables a usar

	char longe[2]="L";
	char shorte[2]="S";
	char operation_ant[2]="L";
	float limit_ant= 0.0;
	float target_ant= 0.0;
	float limit_qty_rng= limit_qty;          // from arguments
	float target_qty_rng= target_qty;       // from arguments
	float axis_size = 0.0;
	float tic = tic_qty;                     // from arguments
	int contract_qty= 1;
	int multiplier= atoi(g_multiplier);

// Definicion de variables para la operación

	float longe_accum = 0.0;  
    float shorte_accum = 0.0;  
    float result_accum_amt = 0.0;  
    float result_accum_pts = 0.0;  
    float limit_nxt = 0 - limit_qty_rng * axis_size;  
    float target_nxt = 0 + target_qty_rng * axis_size;  
    char operation[2] = "L";
    char operation_nxt[2] = "L";
    int target_reached = 0;  
    float range_trail = 0.0;
    float longe_result = 0.0;
    float shorte_result = 0.0;
    float limit = 0.0;
    float target = 0.0;
    int continue_no_change = 0;


//  locate the initial position

    my_day = 2;
    snprintf(my_date_ini, sizeof my_date_ini, "%s%s%d", ini_year, "010", my_day);
 
 	while ((calculaDiaSemana(my_date_ini) == 0) || (calculaDiaSemana(my_date_ini) == 6))
 	{
 		my_day = my_day + 1;
 		snprintf(my_date_ini, sizeof my_date_ini, "%s%s%d", ini_year, "010", my_day);
 	}

 	printf("%s\n", my_date_ini);

 	pt_ini=posicionarseEnUnaFecha_EstructOperativa(my_date_ini);
 

//  locate the final position

	my_day = 31;
    snprintf(my_date_end, sizeof my_date_end, "%s%s%d", end_year, "12", my_day);
 
 	while ((calculaDiaSemana(my_date_ini) == 0) || (calculaDiaSemana(my_date_ini) == 6))
 	{
 		my_day = my_day - 1;
 		snprintf(my_date_end, sizeof my_date_end, "%s%s%d", end_year, "12", my_day);
 	}

 	printf("%s\n", my_date_end);

 	pt_end=posicionarseEnUnaFecha_EstructOperativa(my_date_end);

 	printf("%d - %d\n", pt_ini, pt_end );

 // the loop

 	cont=pt_ini;

 	while (cont <= pt_end)   // bucle
	{
		range_trail = datosdiarios[cont].range_trail;
		longe_result = datosdiarios[cont].longe_rst;
		shorte_result = datosdiarios[cont].shorte_rst;

		axis_size = range_trail;

		if (cont == pt_ini)
		{
			limit_nxt = 0 - limit_qty_rng * axis_size;
            target_nxt = 0 + target_qty_rng * axis_size;
		}

		strcpy(operation, operation_nxt);
		limit = limit_nxt;
		target = target_nxt;

		longe_accum = longe_accum + longe_result;
		shorte_accum = shorte_accum + shorte_result;

		continue_no_change=1;

		if (strcmp(operation, longe)==0)
		{
			result_accum_pts = result_accum_pts + longe_result;                                              // tic commision only for results in dollars not in pts
            result_accum_amt = result_accum_amt + (longe_result - tic) * contract_qty * multiplier;

            if ((longe_accum < limit) && (continue_no_change == 1))                            // overcome the limit and change to shorts
            {
            	strcpy(operation_ant, longe);
                strcpy(operation_nxt, shorte);
                limit_ant = limit;
                limit_nxt = shorte_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = shorte_accum + (target_qty_rng * axis_size);
                continue_no_change = 0;
            }

            if ((target_reached == 1) && (longe_result <= ((0-1) * range_trail * 0.2)) && (continue_no_change == 1))        // target reached with reverse of 20%
            {
            	strcpy(operation_ant, longe);
                strcpy(operation_nxt, shorte);
                limit_ant = limit;
                limit_nxt = shorte_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = shorte_accum + (target_qty_rng * axis_size);
                target_reached = 0;
                continue_no_change = 0;
            }

            if ((longe_accum > target) && (target_reached == 0) && (continue_no_change == 1))                        // target reached but waiting reverse)
            {
            	target_reached = 1;
                strcpy(operation_ant, operation);
                strcpy(operation_nxt, operation_ant);
                limit_ant = limit;
                limit_nxt = limit_ant;
                target_ant = target;
                target_nxt = target_ant;
                continue_no_change = 0;
            }

		}

		if (strcmp(operation, shorte)==0)
		{
			result_accum_pts = result_accum_pts + shorte_result;                                          // tic commision only for results in dollars not in pts
            result_accum_amt = result_accum_amt + (shorte_result - tic) * contract_qty * multiplier;

            if ((shorte_accum < limit) && (continue_no_change == 1))                            // overcome the limit and change to long
            {
            	strcpy(operation_ant, shorte);
                strcpy(operation_nxt, longe);
                limit_ant = limit;
                limit_nxt = longe_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum + (target_qty_rng * axis_size);
                continue_no_change = 0;
            }

            if ((target_reached == 1) && (shorte_result <= ((0-1) * range_trail * 0.2)) && (continue_no_change == 1))        // target reached with reverse of 20%
            {
            	strcpy(operation_ant, shorte);
                strcpy(operation_nxt, longe);
                limit_ant = limit;
                limit_nxt = longe_accum - (limit_qty_rng * axis_size);
                target_ant = target;
                target_nxt = longe_accum + (target_qty_rng * axis_size);
                target_reached = 0;
                continue_no_change = 0;
            }

            if ((shorte_accum > target) && (target_reached == 0) && (continue_no_change == 1))                        // target reached but waiting reverse)
            {
            	target_reached = 1;
                strcpy(operation_ant, operation);
                strcpy(operation_nxt, operation_ant);
                limit_ant = limit;
                limit_nxt = limit_ant;
                target_ant = target;
                target_nxt = target_ant;
                continue_no_change = 0;
            }
        }
        
        if (continue_no_change == 1)  // nothing happen, no target reached, no limit overcame, continue the same
        {
            strcpy(operation_ant, operation);
         	strcpy(operation_nxt, operation_ant);
          	limit_ant = limit;
           	limit_nxt = limit_ant;
           	target_ant = target;
           	target_nxt = target_ant;
        }


 // # save the daily operation

        strcpy(datosdiarios[cont].operation, operation_ant);
        datosdiarios[cont].target = target_ant;
        datosdiarios[cont].limit = limit_ant;
        datosdiarios[cont].result_pts = result_accum_pts - datosdiarios[cont-1].result_accum_pts;
        datosdiarios[cont].result_accum_pts = result_accum_pts;
        datosdiarios[cont].contract_qty = contract_qty;
        datosdiarios[cont].multiplier = multiplier;
        datosdiarios[cont].result_amt = result_accum_amt - datosdiarios[cont-1].result_accum_amt;
        datosdiarios[cont].result_accum_amt = result_accum_amt;
        datosdiarios[cont].longe_accum_tst = longe_accum;
        datosdiarios[cont].shorte_accum_tst = shorte_accum;

    	
        //printf("-->>%d\n", cont);
    	//printf("%s=%s=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f\n",datosdiarios[cont].date_EU,datosdiarios[cont].timetable,
		//			datosdiarios[cont].max,datosdiarios[cont].min,datosdiarios[cont].range,datosdiarios[cont].range_trail,
		//			datosdiarios[cont].range_avg,datosdiarios[cont].open,datosdiarios[cont].longe_close,datosdiarios[cont].shorte_close,
		//			datosdiarios[cont].close,datosdiarios[cont].longe_rst,datosdiarios[cont].shorte_rst,datosdiarios[cont].longe_acc,
		//			datosdiarios[cont].shorte_acc);
    	//printf("%s=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f\n",datosdiarios[cont].operation, datosdiarios[cont].target,datosdiarios[cont].limit,
    	//	datosdiarios[cont].result_pts, datosdiarios[cont].result_accum_pts, datosdiarios[cont].contract_qty, datosdiarios[cont].multiplier,
    	//	datosdiarios[cont].result_amt, datosdiarios[cont].result_accum_amt, datosdiarios[cont].longe_accum_tst, datosdiarios[cont].shorte_accum_tst);

    	cont = cont + 1;

    }

    return 3;

// Fin del código
}

int escribeResultadoTecnicaEX(char mifichero[])
{
    int retorno, es=0;
    FILE *U1;
    U1=fopen(mifichero, "w");
    if (U1 !=NULL)
        {   
            fprintf(U1,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,","date","timetable","max","min","range","range-trail","range-avg",
            	"open","long-close","short-close","close","long-rst","short-rst","longe-acc","shorte_acc","operation","target","limit","result-pts","result-accum-pts",
            	"contract-qty", "multiplier", "result-amt", "result-accum-amt");
            fprintf(U1,"%s,%s\n","longe_accum_tst","shorte_accum_tst");

            while(strlen(datosdiarios[es].date_EU)>8)
            {
               
                fprintf(U1,"%s,%s,%8.2f,%8.2f,%8.2f,%8.2f,",datosdiarios[es].date_EU,datosdiarios[es].timetable,datosdiarios[es].max,datosdiarios[es].min,datosdiarios[es].range,datosdiarios[es].range_trail);
				fprintf(U1,"%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,",datosdiarios[es].range_avg,datosdiarios[es].open,datosdiarios[es].longe_close,datosdiarios[es].shorte_close,datosdiarios[es].close);
				fprintf(U1,"%8.2f,%8.2f,%8.2f,%8.2f,",datosdiarios[es].longe_rst,datosdiarios[es].shorte_rst,datosdiarios[es].longe_acc,datosdiarios[es].shorte_acc);
                fprintf(U1,"%s,%8.2f,%8.2f,%8.2f,%8.2f,",datosdiarios[es].operation,datosdiarios[es].target,datosdiarios[es].limit,datosdiarios[es].result_pts,datosdiarios[es].result_accum_pts);
                fprintf(U1,"%8.2f,%8.2f,%8.2f,%8.2f,",datosdiarios[es].contract_qty,datosdiarios[es].multiplier,datosdiarios[es].result_amt,datosdiarios[es].result_accum_amt);
                fprintf(U1,"%8.2f,%8.2f\n",datosdiarios[es].longe_accum_tst, datosdiarios[es].shorte_accum_tst);

                //fprintf(U1,"%s;%s;%s;",datosdiarios[es].fecha_EU,datosdiarios[es].hora_ini,datosdiarios[es].hora_fin);
                //fprintf(U1,"%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;",datosdiarios[es].apertura,datosdiarios[es].cierrelargos,datosdiarios[es].cierrecortos,datosdiarios[es].cierremercado,datosdiarios[es].maximo,datosdiarios[es].minimo,datosdiarios[es].maximo-datosdiarios[es].minimo,datosdiarios[es].rangotrail);
                //fprintf(U1,"%3d;%8.2f;%8.2f;%8.2f;%8.2f;%s;%3d;%5d;%8.2f;%8.2f\n",datosdiarios[es].altavolatilidad,datosdiarios[es].acumulalargos,datosdiarios[es].acumulacortos,datosdiarios[es].objetivo,datosdiarios[es].limite,datosdiarios[es].operacion,datosdiarios[es].cantidadcontratos,datosdiarios[es].multiplicador,datosdiarios[es].resultado,datosdiarios[es].resultadoacumulado);
                es++;

            }
            retorno=5;
        }
    else
        {   retorno=0;}
    fclose(U1);
    return retorno;
}

int leerFicheroOriginal(char mifichero[]) 
	// leemos y cargamos la información del fichero /Users/agustin/Documents/CDoc/Futuros//TratamientoDatosIntradia/YM.txt **********
	// ***********************************************************************************************
{
    // Formato fechas del fichero de importación = AAAA-MM-DD

	int li=0, lee=0, erro=1, enteroFecha=0, enteroIniFecha, enteroFinFecha, flag;
	char linea[1024], Fecha[10], receptor[25];

	
	enteroIniFecha=convCadenaEntero("20201010");
	enteroIniFecha=20000101;							// Eliminamos el filtro de fechas 

	enteroFinFecha=convCadenaEntero("20201010");
	enteroFinFecha=20990101;


	printf("Iniciamos la carga del fichero %s\n", mifichero);

	FILE *U;
	U=fopen(mifichero, "r");
	if (U==NULL)
	{
		printf("\nERROR: it is impossible open the file = %s\n", mifichero);
		erro=0;
	}
	else
	{
		//printf("Iniciamos la lectura del fichero\n");
		int numerodelimita=0, pos=0, posi=0, k=0;
		int campo, entero=0;
		char delimitador=',';
		while(fgets(linea, 1024, U))
		{

			if (linea[0]=='2')
			{
				// filtro cada línea por fechas
				Fecha[0]=linea[0];
				Fecha[1]=linea[1];
				Fecha[2]=linea[2];
				Fecha[3]=linea[3];
				Fecha[4]=linea[5];
				Fecha[5]=linea[6];
				Fecha[6]=linea[8];
				Fecha[7]=linea[9];
				enteroFecha=convCadenaEntero(Fecha);
				flag=1;
				//printf("EnteroFecha=%d\n", enteroFecha );	
			}
			else
			{
				flag=0;
			}


			if (enteroFecha >= enteroIniFecha && enteroFecha <= enteroFinFecha && flag==1)
			{

				campo=0;						//contador de campos a registrar informacion (nombre,edad,alias,nacionalidad)
				k=0;							//contador de posiciones tratadas de una línea leida del fichero
				posi=0;							//contador de posiciones de los campos del universo
				while(k<strlen(linea) && strlen(linea)>1)
				{
					if(linea[k]==delimitador || linea[k]=='\n')
					{
						if (campo==0)
						{
							strcpy(datosdiarios[lee].date_EU, receptor);       // date
						}
						else if (campo==1)
						{
							strcpy(datosdiarios[lee].timetable, receptor);            // timetable
						}
						else if (campo==2)
						{
							datosdiarios[lee].max=strtof(receptor,NULL);                //max
						}
						else if (campo==3)
						{
							datosdiarios[lee].min=strtof(receptor,NULL);             // min
						}
						else if (campo==4)
						{
							datosdiarios[lee].range=strtof(receptor,NULL);            // range
						}
						else if (campo==5)
						{
							datosdiarios[lee].range_trail=strtof(receptor,NULL);         //range-trail
						}
						else if (campo==6)
						{
							datosdiarios[lee].range_avg=strtof(receptor,NULL);          // range-avg
						}
						else if (campo==7)
						{
							datosdiarios[lee].open=strtof(receptor,NULL);          // open
						}
						else if (campo==8)
						{
							datosdiarios[lee].longe_close=strtof(receptor,NULL);            // longe-close
						}
						else if (campo==9)
						{
							datosdiarios[lee].shorte_close=strtof(receptor,NULL);           // shorte-close
						}
						else if (campo==10)
						{
							datosdiarios[lee].close=strtof(receptor,NULL);             // close
						}
						else if (campo==11)
						{
							datosdiarios[lee].longe_rst=strtof(receptor,NULL);               // longe-rst
						}
						else if (campo==12)
						{
							datosdiarios[lee].shorte_rst=strtof(receptor,NULL);              // shorte-rst
						}
						else if (campo==13)
						{
							datosdiarios[lee].longe_acc=strtof(receptor,NULL);            // longe-acc
						}
						else if (campo==14)
						{
							datosdiarios[lee].shorte_acc=strtof(receptor,NULL);             // shorte-acc
						}
						campo++;
						posi=-1;
					}
					else
					{
						receptor[posi]=linea[k];     // date
						receptor[posi+1]='\0';
					} 
					k++;
					posi++;
				}
				//printf("%d:>> %s",lee, linea);
				//printf("%s=%s=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f=%f\n",datosdiarios[lee].date_EU,datosdiarios[lee].timetable,
				//	datosdiarios[lee].max,datosdiarios[lee].min,datosdiarios[lee].range,datosdiarios[lee].range_trail,
				//	datosdiarios[lee].range_avg,datosdiarios[lee].open,datosdiarios[lee].longe_close,datosdiarios[lee].shorte_close,
				//	datosdiarios[lee].close,datosdiarios[lee].longe_rst,datosdiarios[lee].shorte_rst,datosdiarios[lee].longe_acc,
				//	datosdiarios[lee].shorte_acc);
				lee++;						//contador de lineas leidas del fichero (desde la cero)

			}
		}
	}
	fclose(U);

	printf("Fichero %s leido y procesado\n", mifichero);

	return erro;

}

int convCadenaEntero(char cadena[])
{
	int entero=0, j=0, numero;
	int tamanio=strlen(cadena);
	while (cadena[j]>='0' && cadena[j]<='9' && j<tamanio)   // cuando encuentra un espacio en blanco termina
	{
		numero=cadena[j]-'0';
		entero=entero*10+numero;
		//printf("%d = %d\n",j,entero );
		j++;
	}
	return entero;
}

char *convEnteroCadena(int entero)
{
	static char minuevacadena[10];      // definido como static para poder usarlo en una función
	char nuevacadena[10];
	char digito;
	int conta=0, numero=0, longi=0;

	while (entero/10!=0 || entero>0)   // Aqui saco los números de unidades, decenas, centenas, millares, diezmillares, ..
	{									// en el orden dado.
		numero=entero%10;

		switch (numero)
		{
		case 0: digito='0'; break;
		case 1: digito='1'; break;
		case 2: digito='2'; break;
		case 3: digito='3'; break;
		case 4: digito='4'; break;
		case 5: digito='5'; break;
		case 6: digito='6'; break;
		case 7: digito='7'; break;
		case 8: digito='8'; break;
		case 9: digito='9'; break;
		}

		nuevacadena[conta]=digito;
		entero=entero/10;
		conta++;
	}
	nuevacadena[conta]='0';							// añadimos dos ceros para evitar que devueva cadenas de menos
	nuevacadena[conta+1]='0';						// de dos dígitos
	nuevacadena[conta+2]='\0';
	
	longi=conta;

	if (longi<2)									//Para el caso de un solo dígito o sea cero el entero forzamos a
		{conta=1;longi=1;}							// devolver una cadenas de al menos dos caracteres.
	else
		{conta--;}

	//printf("%s\n", nuevacadena );

	for (int i = 0; i <longi+1; ++i)    // Aquí cambio el orden dado al orden natural centenas, decenas, unidades, ...
	{
		minuevacadena[i]=nuevacadena[conta];
		conta--;
	}

	
//printf("%s\n", minuevacadena );

	return minuevacadena;
}

float ConvCadenaFloat(char cadena[])
{
	float decimal=0.0;
	int j=0, jj=0, numero=0, numeroentero=0, numerodecimal=0, menospos=-1, puntopos=-1;
	int signopositivo=1;
	int parteentera1, parteentera2, partedecimal1, partedecimal2;

	int tamanio=strlen(cadena);

	while (j<tamanio)	// elimino los caracteres que no sea números, puntos o signo negativo.
	{
		if (cadena[j]!='0' && cadena[j]!='1' && cadena[j]!='2' && cadena[j]!='3' && cadena[j]!='4' && cadena[j]!='5' && cadena[j]!='6' && cadena[j]!='7' && cadena[j]!='8' && cadena[j]!='9' && cadena[j]!='.' && cadena[j]!='-')
		{
			jj=j;
			while (jj<tamanio)
			{
				cadena[jj]=cadena[jj+1];
				jj++;
			}
			cadena[jj-1]='\0';
			tamanio=tamanio-1;
			j=j-1;
		}
		j++;
	}

	//printf("cadena=%s\n", cadena);
										// localizo el signo menos y el punto decimal
	j=0;
	while (((cadena[j]>='0' && cadena[j]<='9') || (cadena[j]=='.' || cadena[j]=='-')) && j<tamanio)
	{
		if(cadena[j]=='.')
			{puntopos=j;}
		if(cadena[j]=='-')
			{menospos=j;}
		j++;
	}
	if (menospos==-1 || menospos==0)          // comprobamos el signo positivo, negativo o error de formato
	{
		if (menospos==-1) {signopositivo=1;}       // positivo
		if (menospos==0) {signopositivo=-1;}        // negativo
	}
	else
	{
		signopositivo=2;  //error porque el signo negativo aparece en un lugar distinto al primero.
	}

	if (j==tamanio && signopositivo<2) // comprobamos que tiene todo como se espera (punto, signo y numeros)
	{
		if(puntopos>-1 && signopositivo==1)     // (+) hay punto decimal y hay parte entera
		{
			parteentera1=0;
			parteentera2=puntopos-1;
			partedecimal1=puntopos+1;
			partedecimal2=puntopos+tamanio-parteentera2-2;
		}
		if(puntopos>-1 && signopositivo==-1)     // (-) hay punto decimal y hay parte entera
		{
			parteentera1=1;
			parteentera2=puntopos-1;
			partedecimal1=puntopos+1;
			partedecimal2=puntopos+tamanio-parteentera2-2;
		}
		if(puntopos==-1 && signopositivo==1)     // (+) no hay punto decimal y solo hay parte entera
		{
			parteentera1=0;
			parteentera2=tamanio-1;
			partedecimal1=-1;
			partedecimal2=-1;
		}
		if(puntopos==-1 && signopositivo==-1)     // (-) no hay punto decimal y solo hay parte entera
		{
			parteentera1=1;
			parteentera2=tamanio-1;
			partedecimal1=-1;
			partedecimal2=-1;
		}
	}
	else
	{
		signopositivo=2;    // error
	}

	j=0;
	while (((cadena[j]>='0' && cadena[j]<='9') || (cadena[j]=='.' || cadena[j]=='-')) && j<tamanio && signopositivo<2)   // cuando encuentra un espacio en blanco termina
	{
		if (j>=parteentera1 && j<=parteentera2)
		{
			numero=cadena[j]-'0';
			numeroentero=numeroentero*10+numero;
		}
		if (j>=partedecimal1 && j<=partedecimal2)
		{
			numero=cadena[j]-'0';
			numerodecimal=numerodecimal*10+numero;
		}
		j++;
	}

	//printf("Entero=%d\n", numeroentero);
	//printf("Decimal=%d\n", numerodecimal);
	//printf("%d\n", partedecimal2-partedecimal1+1);
	//printf("%f\n",pow(10,(partedecimal2-partedecimal1+1)));

	if (signopositivo<2)
	{
		decimal=(numeroentero+(numerodecimal/pow(10,(partedecimal2-partedecimal1+1))))*signopositivo;
		return decimal;	
	}
	else
		return 99.999;

}

int posicionarseEnUnaFecha_EstructOperativa(char mifecha[])                          // en formato AAAAMMDD
{

	char cadenaano[5], cadenames[3], cadenadia[3], cadenahora[3], cadenaminu[3], Fecha_EU[11];
	int FechaAnt=0, Fecha=0, miFecha=0, numeroano=0, numeromes=0, numerodia=0, pos=-1, last=0;

	miFecha=convCadenaEntero(mifecha);

	for (int i = 0; strlen(datosdiarios[i].date_EU)>0; ++i)
	{
		cadenaano[0]=datosdiarios[i].date_EU[0];			// compruebo la fecha que es, para ver cuando           // formato AAAA-MM-DD
		cadenaano[1]=datosdiarios[i].date_EU[1];			// cambio de fecha
		cadenaano[2]=datosdiarios[i].date_EU[2];
		cadenaano[3]=datosdiarios[i].date_EU[3];
		cadenaano[4]='\0';
		cadenames[0]=datosdiarios[i].date_EU[5];
		cadenames[1]=datosdiarios[i].date_EU[6];
		cadenames[2]='\0';
		cadenadia[0]=datosdiarios[i].date_EU[8];
		cadenadia[1]=datosdiarios[i].date_EU[9];
		cadenadia[2]='\0';
		numeroano=convCadenaEntero(cadenaano);
		numeromes=convCadenaEntero(cadenames);
		numerodia=convCadenaEntero(cadenadia);
		Fecha=numeroano*10000+numeromes*100+numerodia;

		if ((Fecha==miFecha) || (Fecha == miFecha+1) || (Fecha == miFecha+2))
		{
			pos=i;
			break;
		}
		//printf("%d == %d \n", Fecha, miFecha );
		last = i;
	}

	printf("mi-pos =%d\n", pos);

	if (pos == -1)                                         // no ha encontrado nada y devolvemos la última posicion de la estructura
		pos = last;

	return pos;

}

int calculaDiaSemana(char fecha[])
{
// El formato de la fecha es string = AAAAMMDD

//Declaracion de vectores y variables

int d,m,a,dia_semana,A,B,C,D;
char d_c[6], m_c[6], a_c[6];
int mimesre[]={0,3,3,6,1,4,6,2,5,0,3,5};
int mimesbi[]={0,3,4,0,2,5,0,3,6,1,4,6};

//Para simplificar el algoritmo vamos a dividirlo en 5 coeficientes o partes más pequeñas

d_c[0]=fecha[6];
d_c[1]=fecha[7];
d_c[3]='\0';
m_c[0]=fecha[4];
m_c[1]=fecha[5];
m_c[3]='\0';
a_c[0]=fecha[0];
a_c[1]=fecha[1];
a_c[2]=fecha[2];
a_c[3]=fecha[3];
a_c[4]='\0';

a=convCadenaEntero(a_c);
m=convCadenaEntero(m_c);
d=convCadenaEntero(d_c);

A=(a-1)%7;
B=((a-1)/4-(3*((a-1)/100+1)/4))%7;
C=d%7;

if (((a%4 == 0) && (a%100 !=0)) || (a%400 == 0))
{
	D=mimesbi[m-1];
}
else
{
	D=mimesre[m-1];
}

dia_semana=(A+B+C+D)%7;

//printf("%d\n", dia_semana );
//printf("%d %d %d \n", a,m,d);

//resuelto el dia.

// Domingo=0, Lunes=1, Martes=2, Miercoles=3, Jueves=4, Viernes=5, Sabado=6

return dia_semana;

}

int ComprobarFicheroExiste(char mifichero[])
{
	FILE *fichero;
	fichero = fopen(mifichero, "r");
	if(fichero != NULL)
	{
		fclose(fichero);
		return 1;
	}
	else
	{
		fclose(fichero);
		return 0;
	}
}

char *MiDirectorioActual()
{
   static char cwd[200];
   if (getcwd(cwd, sizeof(cwd)) != NULL) 
   {
       //printf("Current working dir: %s\n", cwd);
   } 
   else 
   {
       //perror("getcwd() error");
       return "error";
   }
   return cwd;
}
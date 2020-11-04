// DESARROLLO PARA GESTIONAR LOS DATOS INTRADIA COMPRADO EN KIBOT
// Con un menú nos pedirá por orden:
//		1. El nombre del contrato (fichero.txt)
//		2. El camino o ruta (saldrá una por omisión?)
//		3. La Fecha inicial del estudio.
//		4. La Fecha final del estudio. 
//		5. La hora de entrada al mercado
//		6. La hora de salida del mercado
//		7. El rango trail con el que trabajar.
//
// Salida de ficheros 
//      1. Fichero terminado en =EU con los datos filtrados y a tratar en formato Europeo
//      2. Fichero terminado en =P0 con el resultado de la operación de futuros
//      3. Fichero terminado en =P1 con el detalle del resultado de la operación de futuros
//
// El objetivo es capturar la información del fichero origen y volcarlo a una estructura de datos para 
// convertir el formato de fecha US a EU y sumarle 6h para cambiar al horario europeo.
//  0- Recogemos la información de un fichero de datos F,H,A,M,m,C,V con delimitador "," coma
//  1- Transformar la fecha y hora en horario europeo
//  2- Preparar los datos de cotización en float para poder hacer cálculos.
//  3- Aplicamos un filtro de fechas para sacar información entre dos fechas.
//  4- Exportamos los datos filtrados a un fichero de texto terminado el nombre en *=EU.txt
//  5- Con el rango de fechas, horario a operar y rango trail, lo aplico y simulo la operativa de futuros.
// Despues con unos datos previos (horario, rango-trail y rango de fechas) sacar la información
//   histórica de haber operado con el instrumento financiero correspondiente como si hubiesemos
// operado en largos y cortos sacando la información (fecha, acum largos, largos, cortos, acum cortos)

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

struct barra                    // Estructura de datos para la información del fichero original de KIBOT
{
	char fecha_US[11];				// Aunque con 10 digitos tendría suficiente pongo 11 para solucionar problema	
	char hora_inicial_US[6];		//  durante la captura de datos del fichero, y poner el caracter '\0'
	char fecha_EU[11];
	char hora_inicial_EU[6];
	char ch_apertura[8];			// Valores en formato caracter tal como ha sido capturado del fichero
	char ch_maximo[8];
	char ch_minimo[8];
	char ch_cierre[8];
	char ch_volumen[8];
	float apertura;					// Valores cambiados del formato caracter a float o entero
	float maximo;
	float minimo;
	float cierre;
	int volumen;
	float entrada;		// A partir de aquí guardaremos los valores de operar desde la hora_ini hasta la hora_fin
	float stoplargo;	// son los valores que iremos recogiendo de la operación por cada intervalo.
	float stopcorto;	// nos servirá para controlar y depurar el código del programa.
	float salidalargo;
	float salidacorto;
	int operarlargo;
	int operarcorto;
	float maximointra;
	float minimointra;
	float rangotrail;
	float tic;             // este es el mínimo movimiento del indice (ES=0.25 CL=0.01)
	char hora_ini[6];
	char hora_fin[6];
	int altavolatilidad;	// pondremos 1 o 0 segun sea el rango de la barra > al rangotrail definido
};

struct operativa 				// Estructura de datos para la información resultante de operar con futuros 
{
	char fecha_EU[11];
	float apertura;
	float cierrelargos;
	float cierrecortos;
	float cierremercado;        // donde cierra el mercado a la hora_fin
	float maximo;
	float minimo;
	char hora_ini[6];
	char hora_fin[6];
	float rangotrail;
	int altavolatilidad;	// aquí pondremos si hay alta volatilidad en la primera barra del día donde se entra.
	float acumulalargos;	// a partir de aquí la información para la tecnica EX
	float acumulacortos;
    float objetivo;
    float limite;
    char operacion[2];
    int cantidadcontratos;
    int multiplicador;
    float resultadoacumulado;
};

struct calcula_rangos 				// Estructura de datos para hacer calculos de rangos y estimarlos 
{
	char fecha_EU[11];
	float r_apertura;
	float r_cierre;
	float r_maximo;
	float r_minimo;
	float r_rango;
	float r_rangotrail;
	char hora_ini[6];
	char hora_fin[6];
};

struct barra datosintradia[374400];  // son 15 años de datos de barras de 15 min. (4x24x5x52x15)
struct operativa datosoperativa[3900]; // son 15 años de datos de operaciones diarias (5x52x15)
struct calcula_rangos datosrangos[3900]; // son 15 años de datos de operaciones diarias (5x52x15)

int ComprobarFicheroExiste(char mifichero[]);
int leerFicheroOriginal(struct barra datos[], char mifichero[], char ini_fecha[], char fin_fecha[]);
int escribeFicheroDatosEU(struct barra datos[], char mifichero[]);
int convCadenaEntero(char cadena[]);
char *convEnteroCadena(int entero);
int operativaFuturos(struct operativa mioperativa[], float rango_trail, float tic_mov, char hora_ini[], char hora_fin[]);
int cambioFormatoUS_EU(struct barra datos[]);
int escribeFicheroResultadoOperativa(struct operativa mioperativa[], char mifichero[]);
int escribeFicheroDetalleOperativa(struct barra datos[], char mifichero[]);
int FechaHoraMinutoDelSistema();
int leerFicheroInput(char mifichero[]);
int HoraValida(int hora, int minutos);				
int AnyoBisiesto(int anyo);
int FechaValida(int dia, int mes, int anyo);
float ConvCadenaFloat(char cadena[]);
char *MiDirectorioActual();                 // devuelve el directorio de trabajo para la gestion de ficheros
int calculaRangosParaUsar(char hora_ini[], char hora_fin[]);
int posicionarseEnUnaFecha_EstructRangos(char mifecha[]);
int posicionarseEnUnaFecha_EstructOperativa(char mifecha[]);
int calculaDiaSemana(char fecha[]);
int escribeFicheroResultadoRangos(char mifichero[]);
int OperacionTecnicaEX(char ini_fecha[], char fin_fecha[]);
int escribeResultadoTecnicaEX(char mifichero[]);
int redondeoSegunContrato(float mi_numerito);

//Variables globales y valores iniciales en MAYUSCULAS.

char MIRUTA[100];                                  // *** ruta de ficheros
char MIINPUT[100]="OperaFuturos=Input.txt";        // ***** Fichero a importar informacion  a tratar
char MIFICHERO[100]="OperaFuturos=Input.txt";    
char MICONTRATO[10]="ES";				// ***************** Contrato a analizar
char INIFECHA[10]="20170101";            // *******Fecha Inicio AAAAMMDD********* Fecha inicial
char FINFECHA[10]="20190101";            // *******Fecha final AAAAMMDD********** Fecha final

char HORAINI[6]="09:00";				//  *******Hora inicio operación HH:MM*** Hora inicial para operar
char HORAFIN[6]="18:30";				//  *******Hora final operación HH:MM**** Hora final para operar

char RANGOTRAIL_C[30]="15";			// ************ Rango Trail único ********** rango a aplicar a la operativa
float RANGOTRAIL=0.0;
char TICMOV_C[10]="0.0";              // *********** Tic de mínimo movimiento *************
float TICMOV =0.0;

char TRAILFIJO[200]="2009:100.00";   //************* Coleccion de trails fijos anuales: prioridad sobre estimaciones **************
char NUMSESIONES_C[10]="1020";           //********** Cantidad de sesiones para usar en el cálculo del trail ********
float NUMSESIONES = 1020;
char PORCREDUCCION_C[10] = "70";       //************ es el porcentaje que se usa para la reducción del trail ********
float PORCREDUCCION = 70;
char ACTUALIZATRAIL[10] = "A";         // ******* cada cuanto tiempo (año, mes, semana, dia) actualizo el trail *******
char NUMEJESOBJ_C[10]="3";             // ********* cantidad de ejes objetivos a usar en la tecnica EX *********
float NUMEJESOBJ=3;
char NUMEJESLIM_C[10]="1.00";          // ********* cantidad de eje limite a usar en la técnica EX **********
float NUMEJESLIM=1.00;
char OPERACIONINICIAL[10]="L";       // ******** con qué operación (Largos/Cortos) empezamos   *****************
char MULTIPLICADOR_C[10]="1000";         //******* cual es el multiplicador del contrato ******************
float MULTIPLICADOR=1000;
char CONTRATOSINICIALES_C[10]="1";      // ********* cantidad de contratos con los que empliezo a operar **********
float CONTRATOSINICIALES=1.0;
char FIXEDRATIO[20]="N:5000";        //******** si se aplica o no y la cantidad a superar (S:5000) ***********
char NUMEJESPROTEC_C[10]="5";			// ******* la cantidad de ejes para la protección en caso de pérdidas **********
float NUMEJESPROTEC=1.0;
char VACIO[100];

int main()
{
	int seguir=1, conta=0; 
	int mienterohora=0, mienterodia=0, mienteromes=0, mienteroano=0,cambiodia=0, mipunto=0, micampo=0;
	int modocalculorango=0;

	char micadena[10], mitexto[10], mientero[10], midecimal[10], minuevaruta[100], minuevocontrato[10];
//char FicheroParaImportarUS[100], FicheroParaExportarEU[100];
//char FicheroParaExportarOP0[100], FicheroParaExportarOP1[100];
	int FechaHM;
	char hora1[5], minu1[5],hora2[5],minu2[5],dia1[5],mes1[5],ano1[5],dia2[5],mes2[5],ano2[5];

	char FicheroParaImportarUS[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES.txt";
	char FicheroParaExportarEU[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES=EU.txt";
	char FicheroParaExportarOP0[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES=OP0.txt";
	char FicheroParaExportarOP1[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES=OP1.txt";
	char FicheroParaExportarRT0[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES=RT0.txt";
	char FicheroParaExportarEX0[100]= "/Users/agustin/Documents/CDoc/Futuros/TratamientoDatosIntradia/ES=EX0.txt";

// Leemos e imprimimos los datos del fichero imput
	sprintf(MIRUTA,"%s",MiDirectorioActual());
	sprintf(MIFICHERO,"%s/%s",MIRUTA,MIINPUT);


	if (leerFicheroInput(MIFICHERO)==1)
	{
		FechaHM=FechaHoraMinutoDelSistema();
		sprintf(FicheroParaImportarUS,"%s%s.txt",MIRUTA,MICONTRATO);
		sprintf(FicheroParaExportarEU,"%s%s=%d=EU.txt",MIRUTA,MICONTRATO,FechaHM);
		sprintf(FicheroParaExportarOP0,"%s%s=%d=OP0.txt",MIRUTA,MICONTRATO,FechaHM);
		sprintf(FicheroParaExportarOP1,"%s%s=%d=OP1.txt",MIRUTA,MICONTRATO,FechaHM);
		sprintf(FicheroParaExportarRT0,"%s%s=%d=RT0.txt",MIRUTA,MICONTRATO,FechaHM);
		sprintf(FicheroParaExportarEX0,"%s%s=%d=EX0.txt",MIRUTA,MICONTRATO,FechaHM);

		printf("Procesando con siguientes parámetros:\n");
		printf("<------------------------------------------------->\n");
		printf(" -> Ruta de acceso 			= %s\n",MIRUTA);
		printf(" -> Contrato       			= %s\n",MICONTRATO);
		printf(" -> Fecha inicio   			= %s\n",INIFECHA);
		printf(" -> Fecha fin      			= %s\n",FINFECHA);
		printf(" -> Horario inicio 			= %s\n",HORAINI);
		printf(" -> Horario fin    			= %s\n",HORAFIN);
		printf(" -> Rango Trail unico   	= %s\n",RANGOTRAIL_C);
		printf(" -> Tic min mov    			= %s\n",TICMOV_C);
		printf(" -> Rango Trail fijo    	= %s\n",TRAILFIJO);
		printf(" -> Num sesiones para trail = %s\n",NUMSESIONES_C);
		printf(" -> Porc reducc para trail  = %s\n",PORCREDUCCION_C);
		printf(" -> Actualización del trail = %s\n",ACTUALIZATRAIL);
		printf(" -> Numero de ejes objetivo = %s\n",NUMEJESOBJ_C);
		printf(" -> Numero de ejes limite   = %s\n",NUMEJESLIM_C);
		printf(" -> Operación inicial 		= %s\n",OPERACIONINICIAL);
		printf(" -> Multiplicador 		    = %s\n",MULTIPLICADOR_C);
		printf(" -> Cantidad contratos inic = %s\n",CONTRATOSINICIALES_C);
		printf(" -> Fixed Ratio             = %s\n",FIXEDRATIO);
		printf(" -> Num de ejes proteccion  = %s\n",NUMEJESPROTEC_C);
		printf("<-------------------------------------------------->\n");
	
		printf("%s\n", FicheroParaImportarUS );
		printf("%s\n", FicheroParaExportarEU );
		printf("%s\n", FicheroParaExportarOP0 );
		printf("%s\n", FicheroParaExportarOP1 );
		printf("%s\n", FicheroParaExportarRT0 );
		printf("%s\n", FicheroParaExportarEX0 );

		RANGOTRAIL=ConvCadenaFloat(RANGOTRAIL_C);
		TICMOV=ConvCadenaFloat(TICMOV_C);
		NUMSESIONES=ConvCadenaFloat(NUMSESIONES_C);
		PORCREDUCCION=ConvCadenaFloat(PORCREDUCCION_C);
		NUMEJESOBJ=ConvCadenaFloat(NUMEJESOBJ_C);
		NUMEJESLIM=ConvCadenaFloat(NUMEJESLIM_C);
		MULTIPLICADOR=ConvCadenaFloat(MULTIPLICADOR_C);
		CONTRATOSINICIALES=ConvCadenaFloat(CONTRATOSINICIALES_C);
		NUMEJESPROTEC=ConvCadenaFloat(NUMEJESPROTEC_C);
		TICMOV=ConvCadenaFloat(TICMOV_C);

	}
	else
	{
		seguir=0;
		printf("ERROR en la lectura del fichero de configuración: %s\n", MIFICHERO );

	}

// Comprobamos el tipo de rango-trail a usar: 
//               1. único para todo el periodo (un solo valor para el periodo elejido)
//               2. fijado por el usuario para cada año (un valor por año para el periodo elejido)
//               3. estimado por el programa según la técnica (estima a partir de un número de sesiones previas)

	printf("RANGOTRAIL=%7.2f : TRAILFIJO=%s=\n", RANGOTRAIL, TRAILFIJO );
	
	if (RANGOTRAIL != 0)         // si hay un rango-trail único se usa para todo el periodo (se descarta el resto)
	{
		modocalculorango=0;
		printf("Modo de calculo rango = Fijo para todos los años\n");
	}
	else if (RANGOTRAIL==0 && strcmp(TRAILFIJO, "c") !=0)    // si no hay rango-trai único (=0) se mira si hay información para el TRAILFIJO por año
	{                            //    y para ello comprobaremos que la informacion está con el formato AAAA:999.99,AAAA:999.99;........
		modocalculorango=1;
		printf("Modo de calculo rango = Varios y fijos para cada año\n");
		printf("RANGOTRAIL=%7.2f : TRAILFIJO=%s\n", RANGOTRAIL, TRAILFIJO );
	}
	else if (RANGOTRAIL==0 && strcmp(TRAILFIJO, "c") ==0)	 // si no hay información para el rago-trail único o fijo, calculamos estimación del rango-trail
	{
		modocalculorango=2;
		printf("Modo de calculo rango = Variable en funcion de numero sesiones ( %s ) actualizables ( %s )\n",NUMSESIONES_C, ACTUALIZATRAIL);
	}


// Leemos fichero de datos del contrato sin más y la volcamos a la estructura de datos 'datosintradia'
//seguir=0;

	if (seguir==1)
	{
		if (ComprobarFicheroExiste(FicheroParaImportarUS))
		{
    		printf("Fichero %s existe y lo leemos\n", FicheroParaImportarUS);
    		seguir=leerFicheroOriginal(datosintradia, FicheroParaImportarUS, INIFECHA, FINFECHA);      // existe fichero e intento leer
    		printf("Fichero %s leido y cargada la información a la estructura de datos\n", FicheroParaImportarUS);
		}
		else
		{									                              // no existe y se termina el programa
			printf("Fichero %s no existe y termina el programa\n", FicheroParaImportarUS);
			seguir=0;
		}
	}
// Tratamos los datos de la estructura de datos para ponerlos en el formato adecuado (horario europeo)

	if (seguir==1)
	{
		printf("Procesando la información de la estructura de datos y pasando del horario americano al europeo\n" );
		seguir=cambioFormatoUS_EU(datosintradia);
	}

// Volcamos la información tratada y formateada a formato EU, a un fichero

	if (seguir==1)
	{
		printf("Ya procesada la información y cambiado el formato, iniciamos el volcado a fichero %s\n",FicheroParaExportarEU);
		seguir=escribeFicheroDatosEU(datosintradia, FicheroParaExportarEU);
	}

// Volcamos la información de rango a la estructura para gestionarla

	if (seguir==1)
	{
		printf("Evaluamos el rango según los parámetros iniciales\n");
		seguir=calculaRangosParaUsar(HORAINI, HORAFIN);                  // rellenamos la estructura con datos de rango
	}

	if (seguir==1)
	{
		printf("Imprimimos la estructura de rangos a fichero\n");
		seguir=escribeFicheroResultadoRangos(FicheroParaExportarRT0);      // imprimimos la estructura con datos de rango
	}

// Procesamos la información para comprobar el resultado de operar con futuros

	if (seguir==1)
	{
		printf("Inicio la operación con futuros para las fechas, horario y rango-trail seleccionado\n");
		seguir=operativaFuturos(datosoperativa, RANGOTRAIL, TICMOV, HORAINI, HORAFIN);
	}

// Volcamos la información resultante de operar con futuros a fichero

	if (seguir==1)
	{
		printf("Terminada la operación con futuros, escribimos el resultado a fichero %s\n", FicheroParaExportarOP1);
		seguir=escribeFicheroResultadoOperativa(datosoperativa, FicheroParaExportarOP1);
	}

// Volcamos la información de detalle (cada 15 minutos) de operar con futuros

	if (seguir==1)
	{
		printf("Terminada la operación con futuros, escribimos el detalle del intradía a fichero %s\n", FicheroParaExportarOP0);
		seguir=escribeFicheroDetalleOperativa(datosintradia, FicheroParaExportarOP0);
	}

// Ejecutamos la operación de futuros con la Técnica EX

	if (seguir==1)
	{
		printf("Inicio la operación con futuros de la Técnica EX para las fechas seleccionadas\n");
		seguir=OperacionTecnicaEX(INIFECHA, FINFECHA);
	}

// Volcamos la información de resultados diarios de operar con futuros

	if (seguir==1)
	{
		printf("Terminada la operación con futuros de la Técnica EX, escribimos el resultado a fichero %s\n", FicheroParaExportarEX0);
		seguir=escribeResultadoTecnicaEX(FicheroParaExportarEX0);
	}

	if (seguir!=1)
	{
		printf("Proceso terminado y con errores.\n");
	}

// Fin del main()
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

int leerFicheroOriginal(struct barra datos[], char mifichero[], char ini_fecha[], char fin_fecha[]) 
	// leemos y cargamos la información del fichero /Users/agustin/Documents/CDoc/Futuros//TratamientoDatosIntradia/YM.txt **********
	// ***********************************************************************************************
{
	int li=0, lee=0, erro=1, enteroFecha=0, enteroIniFecha, enteroFinFecha;
	char linea[1024], Fecha[10];

	
	enteroIniFecha=convCadenaEntero(ini_fecha);
	enteroIniFecha=20000101;							// Eliminamos el filtro de fechas 

	enteroFinFecha=convCadenaEntero(fin_fecha);
	enteroFinFecha=20990101;


	printf("Filtramos la información entre %s y %s\n", ini_fecha, fin_fecha);

	FILE *U;
	U=fopen(mifichero, "r");
	if (U==NULL)
	{
		printf("\nERROR: imposible inicializar desde fichero = %s\n", mifichero);
		erro=0;
	}
	else
	{
		int numerodelimita=0, pos=0, posi=0, k=0;
		int campo, entero=0;
		char delimitador=',';
		while(fgets(linea, 1024, U))
		{
			// filtro cada línea por fechas
			Fecha[0]=linea[6];Fecha[1]=linea[7];Fecha[2]=linea[8];Fecha[3]=linea[9];
			Fecha[4]=linea[0];Fecha[5]=linea[1];
			Fecha[6]=linea[3];Fecha[7]=linea[4];
			enteroFecha=convCadenaEntero(Fecha);

			if (enteroFecha >= enteroIniFecha && enteroFecha <= enteroFinFecha)
			{

				campo=0;						//contador de campos a registrar informacion (nombre,edad,alias,nacionalidad)
				k=0;							//contador de posiciones tratadas de una línea leida del fichero
				posi=0;							//contador de posiciones de los campos del universo
				while(k<strlen(linea) && strlen(linea)>1)
				{
					if(linea[k]==delimitador)
						{campo++;posi=-1;}
					else 
						if (campo==0)
							{datosintradia[lee].fecha_US[posi]=linea[k];datosintradia[lee].fecha_US[posi+1]='\0';}
						else if (campo==1)
							{datosintradia[lee].hora_inicial_US[posi]=linea[k];datosintradia[lee].hora_inicial_US[posi+1]='\0';}
						else if (campo==2)
							{datosintradia[lee].ch_apertura[posi]=linea[k];datosintradia[lee].ch_apertura[posi+1]='\0';}
						else if (campo==3)
							{datosintradia[lee].ch_maximo[posi]=linea[k];datosintradia[lee].ch_maximo[posi+1]='\0';}
						else if (campo==4)
							{datosintradia[lee].ch_minimo[posi]=linea[k];datosintradia[lee].ch_minimo[posi+1]='\0';}
						else if (campo==5)
							{datosintradia[lee].ch_cierre[posi]=linea[k];datosintradia[lee].ch_cierre[posi+1]='\0';}
						else if (campo==6 && linea[k]!='\n')			// Eliminamos el salto de linea
							{datosintradia[lee].ch_volumen[posi]=linea[k];datosintradia[lee].ch_volumen[posi+1]='\0';}
					k++;
					posi++;
				}
				datosintradia[lee].fecha_US[10]='\0';
				datosintradia[lee].hora_inicial_US[5]='\0';
				//printf("%d:>> %s",lee, linea);
				//printf("%s=%s=%s=%s=%s=%s=%s\n",datosintradia[lee].fecha_US,datosintradia[lee].hora_inicial_US,datosintradia[lee].ch_apertura,datosintradia[lee].ch_maximo,datosintradia[lee].ch_minimo,datosintradia[lee].ch_cierre,datosintradia[lee].ch_volumen);
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

int cambioFormatoUS_EU(struct barra datos[])
{
	int conta=0, mienterohora=0, mienterodia=0, mienteromes=0, mienteroano=0,cambiodia=0, mipunto=0, micampo=0;
	char micadena[10], mitexto[10], mientero[10], midecimal[10];
	float mifloat=0.001;
	
	for (int i = 0; i < 374400 && strlen(datosintradia[i].fecha_US)>1; ++i)
	{
		cambiodia=0;
		micadena[0]=datosintradia[i].hora_inicial_US[0];
		micadena[1]=datosintradia[i].hora_inicial_US[1];
		micadena[2]='\0';
		mienterohora=convCadenaEntero(micadena);
		mienterohora=mienterohora+6;                                  //añadimos 6 horas al horario americano para pasarlo al europeo
		if (mienterohora>23)
		{ 	mienterohora=mienterohora-24;                             // si pasa de las 00.00 le quitamos 24h y apuntamos para sumar un dia
			cambiodia=1;
		}
		strcpy(mitexto,convEnteroCadena(mienterohora));
		datosintradia[i].hora_inicial_EU[0]=mitexto[0];                             // escribo la HORA europea definitiva
		datosintradia[i].hora_inicial_EU[1]=mitexto[1];
		datosintradia[i].hora_inicial_EU[2]=datosintradia[i].hora_inicial_US[2];
		datosintradia[i].hora_inicial_EU[3]=datosintradia[i].hora_inicial_US[3];
		datosintradia[i].hora_inicial_EU[4]=datosintradia[i].hora_inicial_US[4];
		datosintradia[i].hora_inicial_EU[5]='\0';
		if (cambiodia==1)                                                          // Si hay cambio de día reviso y lo modifico
		{
			micadena[0]=datosintradia[i].fecha_US[3];                             // reviso el día siguiente y mes
			micadena[1]=datosintradia[i].fecha_US[4];
			micadena[2]='\0';
			mienterodia=convCadenaEntero(micadena);
			mienterodia=mienterodia+1;
			micadena[0]=datosintradia[i].fecha_US[0];
			micadena[1]=datosintradia[i].fecha_US[1];
			micadena[2]='\0';
			mienteromes=convCadenaEntero(micadena);
			micadena[0]=datosintradia[i].fecha_US[6];
			micadena[1]=datosintradia[i].fecha_US[7];
			micadena[2]=datosintradia[i].fecha_US[8];
			micadena[3]=datosintradia[i].fecha_US[9];
			micadena[4]='\0';
			mienteroano=convCadenaEntero(micadena);
			if (mienterodia==32 && (mienteromes==1 || mienteromes==3 || mienteromes==5 || mienteromes==7 || mienteromes==8 || mienteromes==10 || mienteromes==12))
			{
				mienterodia=1;
				mienteromes=mienteromes+1;
				if (mienteromes==13)
				{
					mienteromes=1;
					mienteroano=mienteroano+1;
				}
			}
			else if (mienterodia==31 && (mienteromes==4 || mienteromes==6 || mienteromes==9 || mienteromes==11))
			{
				mienterodia=1;
				mienteromes=mienteromes+1;
			}
			else if ((mienterodia==29 || mienterodia==30) && mienteromes==2)
			{
				mienterodia=1;
				mienteromes=mienteromes+1;
			}

			strcpy(mitexto,convEnteroCadena(mienterodia));
			datosintradia[i].fecha_EU[0]=mitexto[0];
			datosintradia[i].fecha_EU[1]=mitexto[1];
			datosintradia[i].fecha_EU[2]=datosintradia[i].fecha_US[2];
			strcpy(mitexto,convEnteroCadena(mienteromes));
			datosintradia[i].fecha_EU[3]=mitexto[0];
			datosintradia[i].fecha_EU[4]=mitexto[1];
			datosintradia[i].fecha_EU[5]=datosintradia[i].fecha_US[5];
			strcpy(mitexto,convEnteroCadena(mienteroano));
			datosintradia[i].fecha_EU[6]=mitexto[0];
			datosintradia[i].fecha_EU[7]=mitexto[1];
			datosintradia[i].fecha_EU[8]=mitexto[2];
			datosintradia[i].fecha_EU[9]=mitexto[3];
			datosintradia[i].fecha_EU[10]='\0';
		}
		else                                                                // Si no hay cambio de día
		{
			datosintradia[i].fecha_EU[0]=datosintradia[i].fecha_US[3];
			datosintradia[i].fecha_EU[1]=datosintradia[i].fecha_US[4];
			datosintradia[i].fecha_EU[2]=datosintradia[i].fecha_US[2];
			datosintradia[i].fecha_EU[3]=datosintradia[i].fecha_US[0];
			datosintradia[i].fecha_EU[4]=datosintradia[i].fecha_US[1];
			datosintradia[i].fecha_EU[5]=datosintradia[i].fecha_US[5];
			datosintradia[i].fecha_EU[6]=datosintradia[i].fecha_US[6];
			datosintradia[i].fecha_EU[7]=datosintradia[i].fecha_US[7];
			datosintradia[i].fecha_EU[8]=datosintradia[i].fecha_US[8];
			datosintradia[i].fecha_EU[9]=datosintradia[i].fecha_US[9];
			datosintradia[i].fecha_EU[10]='\0';
		}

		// Analizamos si son números enteros o llevan decimales buscando el punto decimal (uno a uno)

		micampo=0;
		while (micampo<6)
		{
			micampo++;
			if (micampo==1)
				{strcpy(micadena,datosintradia[i].ch_apertura);}
			else if (micampo==2)
				{strcpy(micadena,datosintradia[i].ch_maximo);}
			else if (micampo==3)
				{strcpy(micadena,datosintradia[i].ch_minimo);}
			else if (micampo==4)
				{strcpy(micadena,datosintradia[i].ch_cierre);}
			else if (micampo==5)
				{strcpy(micadena,datosintradia[i].ch_volumen);}



			mifloat=ConvCadenaFloat(micadena);

			if (micampo==1)
				{
					datosintradia[i].apertura=mifloat;
				}
			if (micampo==2)
				{
					datosintradia[i].maximo=mifloat;
				}
			if (micampo==3)
				{
					datosintradia[i].minimo=mifloat;
				}
			if (micampo==4)
				{
					datosintradia[i].cierre=mifloat;
				}
			if (micampo==5)
				{
					datosintradia[i].volumen=mifloat;
				}

		}


//		printf("%d\n", i);
//		printf("%s;%s;%s;",datosintradia[i].fecha_US,datosintradia[i].hora_inicial_US,datosintradia[i].ch_apertura);
//		printf("%s;%s;%s;%s\n",datosintradia[i].ch_maximo,datosintradia[i].ch_minimo,datosintradia[i].ch_cierre,datosintradia[i].ch_volumen);
//		printf("%s-%s-",datosintradia[i].fecha_EU,datosintradia[i].hora_inicial_EU );
//		printf("%8.2f-%8.2f-%8.2f-%8.2f-%d\n",datosintradia[i].apertura,datosintradia[i].maximo,datosintradia[i].minimo,datosintradia[i].cierre,datosintradia[i].volumen);
			
	}
	return 1;
}

int operativaFuturos(struct operativa mioperativa[], float rango_trail, float tic_mov, char hora_ini[], char hora_fin[])
{
	int numerohoraini, numerominuini, numerohorafin, numerominufin, numerohorarioIni, numerohorarioFin;
	char cadenaano[5], cadenames[3], cadenadia[3], cadenahora[3], cadenaminu[3], Fecha_EU[11];

	float Entrada=0.0, SalidaLargo=0.0, SalidaCorto=0.0, CierreMercado=0.0, StopCorto=0.0, StopLargo=0.0; 
	float Maximo=0.0, Minimo=0.0, Minmov=0.0; 
	int Fecha=0, FechaAnt=0, numeroano=0, numeromes=0, numerodia=0;  //seriales
	int OperarLargo=0, OperarCorto=0; 
	int j=0, i=0, tratados=0, encontrado=0, operando=1;		// contador

	//Minmov = tic_mov;   // Ojo que mejor hacer este ajuste en la aplicación de la técnica EX <<<<<<***************

// Convierto en número entero el horario de operación (hora+minutos inicial y final)

// El horario de operación debe estar dentro del mismo día. No puede cambiar de día.

	cadenahora[0]=hora_ini[0];
	cadenahora[1]=hora_ini[1];
	cadenahora[2]='\0';
	cadenaminu[0]=hora_ini[3];
	cadenaminu[1]=hora_ini[4];
	cadenaminu[2]='\0';
	numerohoraini=convCadenaEntero(cadenahora);
	numerominuini=convCadenaEntero(cadenaminu);
	numerohorarioIni=numerohoraini*100+numerominuini;

	cadenahora[0]=hora_fin[0];
	cadenahora[1]=hora_fin[1];
	cadenahora[2]='\0';
	cadenaminu[0]=hora_fin[3];
	cadenaminu[1]=hora_fin[4];
	cadenaminu[2]='\0';
	numerohorafin=convCadenaEntero(cadenahora);
	numerominufin=convCadenaEntero(cadenaminu);
	numerohorarioFin=numerohorafin*100+numerominufin;

	//printf("%s+%s\n", hora_ini, hora_fin );

// Bucle que recorre toda la estructura de datos

	for (int i = 0; i < 375000 && datosintradia[i].fecha_EU[0]!='\0'; ++i)
	{
		cadenaano[0]=datosintradia[i].fecha_EU[6];			// compruebo la fecha que es, para ver cuando
		cadenaano[1]=datosintradia[i].fecha_EU[7];			// cambio de fecha
		cadenaano[2]=datosintradia[i].fecha_EU[8];
		cadenaano[3]=datosintradia[i].fecha_EU[9];
		cadenaano[4]='\0';
		cadenames[0]=datosintradia[i].fecha_EU[3];
		cadenames[1]=datosintradia[i].fecha_EU[4];
		cadenames[2]='\0';
		cadenadia[0]=datosintradia[i].fecha_EU[0];
		cadenadia[1]=datosintradia[i].fecha_EU[1];
		cadenadia[2]='\0';
		numeroano=convCadenaEntero(cadenaano);
		numeromes=convCadenaEntero(cadenames);
		numerodia=convCadenaEntero(cadenadia);
		Fecha=numeroano*10000+numeromes*100+numerodia;

		if (Fecha > FechaAnt)								// momento que inicia un nuevo día
		{
			FechaAnt=Fecha;
			//printf(">>>>>%d=A=%5.0f, SL=%5.0f, SC=%5.0f, M=%5.0f, m=%5.0f, F=%s,\n",j-1,datosoperativa[j-1].apertura,datosoperativa[j-1].cierrelargos,datosoperativa[j-1].cierrecortos,datosoperativa[j-1].maximo,datosoperativa[j-1].minimo,datosoperativa[j-1].fecha_EU);

										//Busco el rango-trail en la estructura de rangos.
			encontrado=0;
			for (int e = 0; (strlen(datosrangos[e].fecha_EU) > 8 && encontrado==0); ++e)
			{
				if (strcmp(datosrangos[e].fecha_EU,datosintradia[i].fecha_EU)==0)
				{
					rango_trail=datosrangos[e].r_rangotrail;
					encontrado=1;
				}
			}
		}

		cadenahora[0]=datosintradia[i].hora_inicial_EU[0];    //compruebo la hora+minutos Inicial y Final
		cadenahora[1]=datosintradia[i].hora_inicial_EU[1];    // cada registro (i) es de 15 minutos
		cadenahora[2]='\0';
		cadenaminu[0]=datosintradia[i].hora_inicial_EU[3];
		cadenaminu[1]=datosintradia[i].hora_inicial_EU[4];
		cadenaminu[2]='\0';
		numerohoraini=convCadenaEntero(cadenahora);
		numerominuini=convCadenaEntero(cadenaminu);
		numerohorafin=numerohoraini;
		numerohoraini=numerohoraini*100+numerominuini;
		numerominufin=numerominuini+15;
		if (numerominufin==60)								// ojo porque hay que identificar solo hora+minuto
		{													// la fecha no se incluye
			numerominufin=0;								// por ejemplo de 1830 paso a 1845
			numerohorafin=numerohorafin+1;					// de 1845 paso a 1900
			if (numerohorafin==24)							// de 2345 paso a 0000
			{
				numerohorafin=0;
			}

		}
		numerohorafin=numerohorafin*100+numerominufin;

		strcpy(Fecha_EU, datosintradia[i].fecha_EU);

		 // compruebo que la hora inicio barra es >= que la del horario inicial y < que la del horario final
		if (numerohoraini>=numerohorarioIni && numerohoraini<numerohorarioFin)            
		{
			if(numerohoraini==numerohorarioIni && operando==0) 	// Entro en el mercado en la apertura según horario
			{
				Entrada = datosintradia[i].apertura;
            	StopLargo = Entrada - rango_trail;
            	StopCorto = Entrada + rango_trail;
            	SalidaLargo = datosintradia[i].cierre;			// OJO solo vale cuando la primera barra es menor al TRAIL
            	SalidaCorto = datosintradia[i].cierre;			// OJO solo vale cuando la primera barra es menor al TRAIL
            	OperarLargo = 1;
            	OperarCorto = 1;
            	Maximo = datosintradia[i].maximo;
            	Minimo = datosintradia[i].minimo;
            	operando = 1;										// marco que entro al mercado y esto operando 
            	//printf("E=%f\n", Entrada );
			}

			if (operando==1)                                    // entro solo cuando estoy operando.
			{
				if (OperarCorto == 1)
				{
					SalidaCorto = datosintradia[i].cierre + Minmov;
				}

				if (OperarLargo == 1)
				{
					SalidaLargo = datosintradia[i].cierre - Minmov;
				}

				if (datosintradia[i].maximo > StopCorto && OperarCorto==1) // Si supera por arriba el stop cortos = termino cortos
				{
					SalidaCorto = StopCorto + Minmov;
					OperarCorto = 0;
				}

				if (datosintradia[i].minimo < StopLargo && OperarLargo==1) // Si supera por abajo el stop largos = termino largos
				{
					SalidaLargo = StopLargo - Minmov;
					OperarLargo = 0;
				}

				if (StopLargo < (datosintradia[i].maximo - rango_trail) && OperarLargo==1) //Subo stop Largos
				{
					StopLargo = datosintradia[i].maximo - rango_trail;
				}

				if (StopCorto > (datosintradia[i].minimo + rango_trail) && OperarCorto==1)  //Subo stop Cortos
				{
					StopCorto = datosintradia[i].minimo + rango_trail;
				}

				if (numerohorafin==numerohorarioFin && OperarLargo==1) // Salgo del mercado por largos según horario
				{
            		OperarLargo = 0;
				}

				if(numerohorafin==numerohorarioFin && OperarCorto==1) // Salgo del mercado por cortos según horario
				{
            		OperarCorto = 0;
				}

				if (datosintradia[i].maximo > Maximo)		// actualizo máximo si procede
				{
					Maximo = datosintradia[i].maximo;
				}

				if (datosintradia[i].minimo < Minimo)		// actualizo mínimo si procede
				{
					Minimo = datosintradia[i].minimo;
				}
				CierreMercado = datosintradia[i].cierre;
			}
		}

		else if (numerohoraini==numerohorarioFin && operando==1)       // El horario de operación terminó y grabamos resultado
		{
			strcpy(datosoperativa[j].fecha_EU,Fecha_EU);
			datosoperativa[j].apertura=Entrada;
			datosoperativa[j].cierrelargos=SalidaLargo;
			datosoperativa[j].cierrecortos=SalidaCorto;
			datosoperativa[j].cierremercado=CierreMercado;
			datosoperativa[j].maximo=Maximo;
			datosoperativa[j].minimo=Minimo;
			strcpy(datosoperativa[j].hora_ini, hora_ini);
			strcpy(datosoperativa[j].hora_fin, hora_fin);
			datosoperativa[j].rangotrail=rango_trail;
			datosoperativa[j].altavolatilidad=0;
			j=j+1;
													// Como se ha terminado la operación reinicializamos
			Entrada = 0;
            StopLargo = 0;
            StopCorto = 0;
            SalidaLargo = 0;
            SalidaCorto = 0;
            CierreMercado = 0;
            OperarLargo = 0;
            OperarCorto = 0;
            Maximo = 0;
            Minimo = 0;
            operando = 0;
		}

		// vuelco la información de los cálculos a la estructura de datosintradía
		datosintradia[i].entrada=Entrada;
		datosintradia[i].stoplargo=StopLargo;
		datosintradia[i].stopcorto=StopCorto;
		datosintradia[i].salidalargo=SalidaLargo;
		datosintradia[i].salidacorto=SalidaCorto;
		datosintradia[i].operarlargo=OperarLargo;
		datosintradia[i].operarcorto=OperarCorto;
		datosintradia[i].maximointra=Maximo;
		datosintradia[i].minimointra=Minimo;
		datosintradia[i].rangotrail=rango_trail;
		strcpy(datosintradia[i].hora_ini, hora_ini);
		strcpy(datosintradia[i].hora_fin, hora_fin);
		datosintradia[i].altavolatilidad=0;		

		//printf("%d=%d = %04d : %04d = %04d : %04d||E=%5.0f:StL=%5.0f:StC=%5.0f:SL=%5.0f:SC=%5.0f:OL=%d:OC=%d:M=%5.0f:m=%5.0f >> %s:%s\n",i, Fecha, numerohoraini, numerohorafin, numerohorarioIni, numerohorarioFin,Entrada,StopLargo,StopCorto,SalidaLargo,SalidaCorto,OperarLargo,OperarCorto,Maximo,Minimo,hora_ini,hora_fin);
		tratados=i;

	}

	if (tratados>0)
		{ return 1;}
	else
		{ return 0;}

}

int escribeFicheroResultadoOperativa(struct operativa mioperativa[], char mifichero[])
{
	int retorno, es=0;
	FILE *U1;
	U1=fopen(mifichero, "w");
	if (U1 !=NULL)
		{	
			while(strlen(datosoperativa[es].fecha_EU)>8)
			{
				fprintf(U1,"%s;%s;%s;",datosoperativa[es].fecha_EU,datosoperativa[es].hora_ini,datosoperativa[es].hora_fin);
				fprintf(U1,"%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f\n",datosoperativa[es].apertura,datosoperativa[es].cierrelargos,datosoperativa[es].cierrecortos,datosoperativa[es].cierremercado,datosoperativa[es].maximo,datosoperativa[es].minimo,datosoperativa[es].rangotrail);
				es++;
			}
			retorno=1;
		}
	else
		{	retorno=0;}
	fclose(U1);
	return retorno;

}

int escribeFicheroDetalleOperativa(struct barra datos[], char mifichero[])
{
	int retorno, e=0;
	FILE *U2;
	U2=fopen(mifichero, "w");
	if (U2 !=NULL)
		{	
			while(strlen(datosintradia[e].fecha_EU)>8)
			{
				fprintf(U2,"%s;%s;",datosintradia[e].fecha_EU,datosintradia[e].hora_inicial_EU);
				fprintf(U2,"%8.2f;%8.2f;%8.2f;%8.2f;%6d||",datosintradia[e].apertura,datosintradia[e].maximo,datosintradia[e].minimo,datosintradia[e].cierre,datosintradia[e].volumen);
				fprintf(U2,"%s:%s:%8.2f;E=%8.2f;StL=%8.2f;StC=%8.2f;",datosintradia[e].hora_ini,datosintradia[e].hora_fin,datosintradia[e].rangotrail,datosintradia[e].entrada,datosintradia[e].stoplargo,datosintradia[e].stopcorto);
				fprintf(U2,"SL=%8.2f;SC=%8.2f;OL=%d;OC=%d;M=%8.2f;m=%8.2f\n",datosintradia[e].salidalargo,datosintradia[e].salidacorto,datosintradia[e].operarlargo,datosintradia[e].operarcorto,datosintradia[e].maximointra,datosintradia[e].minimointra);
				e++;
			}
			retorno=1;
		}
	else
		{	retorno=0;}
	fclose(U2);
	return retorno;
}


int escribeFicheroDatosEU(struct barra datos[], char mifichero[])
{
	int retorno, es=0;
	FILE *U1;
	U1=fopen(mifichero, "w");
	if (U1 !=NULL)
		{	
			while(strlen(datosintradia[es].fecha_US)>8)
			{
				fprintf(U1,"%s;%s;",datosintradia[es].fecha_EU,datosintradia[es].hora_inicial_EU );
				fprintf(U1,"%8.2f;%8.2f;%8.2f;%8.2f;%d\n",datosintradia[es].apertura,datosintradia[es].maximo,datosintradia[es].minimo,datosintradia[es].cierre,datosintradia[es].volumen);
				es++;
			}
			retorno=1;
		}
	else
		{	retorno=0;}
	fclose(U1);
	return retorno;
}

int FechaHoraMinutoDelSistema()

{
		int MiNumero=0;

        time_t tiempo = time(0);
        struct tm *tlocal = localtime(&tiempo);
        char output[128];
//        strftime(output,128,"%d/%m/%Y %H:%M:%S",tlocal);
        strftime(output,128,"%y%m%d%H%M",tlocal);
        
//        printf("%s\n",output);

        MiNumero=convCadenaEntero(output);
        return MiNumero;
}

int leerFicheroInput(char mifichero[])
{
	int li=0, lee=0, erro=1, enteroFecha=0, enteroIniFecha, enteroFinFecha, numerodelinea=0;
	char linea[350], informacion[250],vacio[100];

	FILE *U;
	U=fopen(mifichero, "r");
	if (U==NULL)
	{
		printf("\nERROR: imposible inicializar desde fichero = %s\n", mifichero);
		erro=0;
	}
	else
	{
		int posi=0, k=0;
		int campo, entero=0;
		char delimitador='=';
		while(fgets(linea, 350, U))
		{
			numerodelinea++;    			//contador de lineas leidas del fichero
			strcpy(informacion,vacio);        // inicializo el campo comun informacion metiendo '\0' en todas sus posiciones
			campo=0;			//contador de campos a registrar informacion
			k=0;				//contador de posiciones tratadas de una línea leida del fichero
			posi=0;				//contador de posiciones de la información recogida en campos
			while(k<strlen(linea) && strlen(linea)>1)
			{
				if(linea[k]==delimitador)
					{campo++;posi=0;}
				else
				{
					if (campo==1)
					{
						informacion[posi]=linea[k];
						informacion[posi+1]='\0';
						posi++;
					}
				}
				k++;
			}

			if (numerodelinea==2)					// linea 2 => ruta
			{
				strcpy(MIRUTA,informacion);
			}
			else if (numerodelinea==3)					// linea 3 => contrato
			{
				strcpy(MICONTRATO,informacion);
			}
			else if (numerodelinea==4)					// linea 4 => fecha inicio
			{
				strcpy(INIFECHA,informacion);
			}
			else if (numerodelinea==5)					// linea 5 => fecha fin
			{
				strcpy(FINFECHA,informacion);
			}
			else if (numerodelinea==6)					// linea 6 => hora inicio
			{
				strcpy(HORAINI,informacion);
			}
			else if (numerodelinea==7)					// linea 7 => hora fin
			{
				strcpy(HORAFIN,informacion);
			}
			else if (numerodelinea==8)					// linea 8 => rangotrail
			{
				strcpy(RANGOTRAIL_C,informacion);
			}
			else if (numerodelinea==9)					// linea 9 => tic minimo movimiento
			{
				strcpy(TICMOV_C,informacion);
			}
			else if (numerodelinea==10)					// linea 10 => Trail fijo para cada año
			{
				strcpy(TRAILFIJO,informacion);
			}
			else if (numerodelinea==11)					// linea 11 => Cantidad de sesiones para calcular el trail
			{
				strcpy(NUMSESIONES_C,informacion);
			}
			else if (numerodelinea==12)					// linea 12 => Porcentaje de reduccion en el calculo del trail
			{
				strcpy(PORCREDUCCION_C,informacion);
			}
			else if (numerodelinea==13)					// linea 13 => Actualización del trail cada (A - M - S - D)
			{
				strcpy(ACTUALIZATRAIL,informacion);
			}
			else if (numerodelinea==14)					// linea 14 => Numero de ejes objetivo (2, 3, 4, 5, 6)
			{
				strcpy(NUMEJESOBJ_C,informacion);
			}
			else if (numerodelinea==15)					// linea 15 => Numero de ejes limite (0.75, 1.00, 1.25, 1.50)
			{
				strcpy(NUMEJESLIM_C,informacion);
			}
			else if (numerodelinea==16)					// linea 16 => Operación inicial (L - C)
			{
				strcpy(OPERACIONINICIAL,informacion);
			}
			else if (numerodelinea==17)					// linea 17 => Multiplicador (50, 20, 1000)   (ES, NQ, CL)
			{
				strcpy(MULTIPLICADOR_C,informacion);
			}
			else if (numerodelinea==18)					// linea 18 => Cantidad de contratos iniciales (1, 2, 3)
			{
				strcpy(CONTRATOSINICIALES_C,informacion);
			}
			else if (numerodelinea==19)					// linea 19 => Fixed ratio (S/N:5000)
			{
				strcpy(FIXEDRATIO,informacion);
			}
			else if (numerodelinea==20)					// linea 20 => Ejes de protección (3, 4, 5, 6)
			{
				strcpy(NUMEJESPROTEC_C,informacion);
			}
			printf("%d=>%s<=\n",numerodelinea, informacion );
		}
	}
	fclose(U);
	
	if (erro==1)
	{
		printf("Fichero de configuracion e imputs >> %s << leido y procesado\n", mifichero);
	}
	
	return erro;
}

int HoraValida(int hora, int minutos)
{
	if ((hora>23 || hora<0) || (minutos>59 || minutos<0))
		{return 0;}
	else
		{return 1;}
}

int AnyoBisiesto(int anyo)
{
	// Devuelve 1 si el año es bisiesto y 0 en caso contrario
	return (((anyo%4 == 0) && (anyo%100 !=0)) || (anyo%400 == 0));
}

int FechaValida(int dia, int mes, int anyo)

{
	int ulti;
	switch (mes)
	{
		case 1: ulti=31; break;
		case 2: if (AnyoBisiesto(anyo)) {ulti=29;}
				else {ulti=28;} 
				break;
		case 3: ulti=31; break;
		case 4: ulti=30; break;
		case 5: ulti=31; break;
		case 6: ulti=30; break;
		case 7: ulti=31; break;
		case 8: ulti=31; break;
		case 9: ulti=30; break;
		case 10: ulti=31; break;
		case 11: ulti=30; break;
		case 12: ulti=31; break;
	}

	return ((mes<=12) && (dia<=ulti));
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

int calculaRangosParaUsar(char hora_ini[], char hora_fin[])
{
	int numerohoraini, numerominuini, numerohorafin, numerominufin, numerohorarioIni, numerohorarioFin;
	char cadenaano[5], cadenames[3], cadenadia[3], cadenahora[3], cadenaminu[3], Fecha_EU[11], cadenafecha[10], cade_names[8];
	int FechaAnt=0, Fecha=0, numeroano=0, numeromes=0, numerodia=0;
	int j=0, i=0, tratados=0, l=0, operando=0;		// contador
	float Entrada=0.0, CierreMercado=0.0; 
	float Maximo=0.0, Minimo=0.0;

// Convierto en número entero el horario de operación (hora+minutos inicial y final)

	cadenahora[0]=hora_ini[0];
	cadenahora[1]=hora_ini[1];
	cadenahora[2]='\0';
	cadenaminu[0]=hora_ini[3];
	cadenaminu[1]=hora_ini[4];
	cadenaminu[2]='\0';
	numerohoraini=convCadenaEntero(cadenahora);
	numerominuini=convCadenaEntero(cadenaminu);
	numerohorarioIni=numerohoraini*100+numerominuini;

	cadenahora[0]=hora_fin[0];
	cadenahora[1]=hora_fin[1];
	cadenahora[2]='\0';
	cadenaminu[0]=hora_fin[3];
	cadenaminu[1]=hora_fin[4];
	cadenaminu[2]='\0';
	numerohorafin=convCadenaEntero(cadenahora);
	numerominufin=convCadenaEntero(cadenaminu);
	numerohorarioFin=numerohorafin*100+numerominufin;

// Bucle que recorre toda la estructura de datos intradia para calcular rango según el horario de operativa

	printf("%s-%s||%d -> %d : recojo el horario inicial y final a filtrar\n",hora_ini, hora_fin, numerohorarioIni, numerohorarioFin );

	for (int i = 0; i < 375000 && datosintradia[i].fecha_EU[0]!='\0'; ++i)
	{
		cadenaano[0]=datosintradia[i].fecha_EU[6];			// compruebo la fecha que es, para ver cuando
		cadenaano[1]=datosintradia[i].fecha_EU[7];			// cambio de fecha
		cadenaano[2]=datosintradia[i].fecha_EU[8];
		cadenaano[3]=datosintradia[i].fecha_EU[9];
		cadenaano[4]='\0';
		cadenames[0]=datosintradia[i].fecha_EU[3];
		cadenames[1]=datosintradia[i].fecha_EU[4];
		cadenames[2]='\0';
		cadenadia[0]=datosintradia[i].fecha_EU[0];
		cadenadia[1]=datosintradia[i].fecha_EU[1];
		cadenadia[2]='\0';
		numeroano=convCadenaEntero(cadenaano);
		numeromes=convCadenaEntero(cadenames);
		numerodia=convCadenaEntero(cadenadia);
		Fecha=numeroano*10000+numeromes*100+numerodia;

		if (Fecha > FechaAnt)								// momento que inicia un nuevo día
		{
			FechaAnt=Fecha;
			//printf(">>>>>%d=A=%5.0f, SL=%5.0f, SC=%5.0f, M=%5.0f, m=%5.0f, F=%s,\n",j-1,datosoperativa[j-1].apertura,datosoperativa[j-1].cierrelargos,datosoperativa[j-1].cierrecortos,datosoperativa[j-1].maximo,datosoperativa[j-1].minimo,datosoperativa[j-1].fecha_EU);
		}

		cadenahora[0]=datosintradia[i].hora_inicial_EU[0];    //compruebo la hora+minutos Inicial y Final
		cadenahora[1]=datosintradia[i].hora_inicial_EU[1];    // cada registro (i) es de 15 minutos
		cadenahora[2]='\0';
		cadenaminu[0]=datosintradia[i].hora_inicial_EU[3];
		cadenaminu[1]=datosintradia[i].hora_inicial_EU[4];
		cadenaminu[2]='\0';
		numerohoraini=convCadenaEntero(cadenahora);
		numerominuini=convCadenaEntero(cadenaminu);
		numerohorafin=numerohoraini;
		numerohoraini=numerohoraini*100+numerominuini;
		numerominufin=numerominuini+15;
		if (numerominufin==60)								// ojo porque hay que identifica solo hora+minuto
		{													// la fecha no se incluye
			numerominufin=0;								// por ejemplo de 1830 paso a 1845
			numerohorafin=numerohorafin+1;					// de 1845 paso a 1900
			if (numerohorafin==24)							// de 2345 paso a 0000
			{
				numerohorafin=0;
			}

		}
		numerohorafin=numerohorafin*100+numerominufin;

		strcpy(Fecha_EU, datosintradia[i].fecha_EU);

		 // compruebo que la hora inicio barra es >= que la del horario inicial y < que la del horario final
		if (numerohoraini>=numerohorarioIni && numerohoraini<numerohorarioFin)            
		{
			if(numerohoraini==numerohorarioIni && operando==0) 	// Entro en el mercado en la apertura según horario
			{
				Entrada = datosintradia[i].apertura;
            	Maximo = datosintradia[i].maximo;
            	Minimo = datosintradia[i].minimo;
            	operando = 1;
            	//printf("E=%f\n", Entrada );
			}

			if (operando==1)
			{
				if (datosintradia[i].maximo > Maximo)		// actualizo máximo si procede
				{
					Maximo = datosintradia[i].maximo;
				}

				if (datosintradia[i].minimo < Minimo)		// actualizo mínimo si procede
				{
					Minimo = datosintradia[i].minimo;
				}
				CierreMercado = datosintradia[i].cierre;
			}
		}

		else if (numerohoraini==numerohorarioFin)       // El horario de operación terminó y grabamos resultado
		{
			
			strcpy(datosrangos[j].fecha_EU,Fecha_EU);
			datosrangos[j].r_apertura=Entrada;
			datosrangos[j].r_cierre=CierreMercado;
			datosrangos[j].r_maximo=Maximo;
			datosrangos[j].r_minimo=Minimo;
			strcpy(datosrangos[j].hora_ini, hora_ini);
			strcpy(datosrangos[j].hora_fin, hora_fin);
			datosrangos[j].r_rango=Maximo-Minimo;

			//printf("%d>>%s=%s=%s||A=%7.2f:C=%7.2f:M=%7.2f:m=%7.2f:R=%7.2f\n",j, datosrangos[j].fecha_EU, datosrangos[j].hora_ini, datosrangos[j].hora_fin, datosrangos[j].r_apertura,datosrangos[j].r_cierre,datosrangos[j].r_maximo,datosrangos[j].r_minimo,datosrangos[j].r_rango);


			j=j+1;
													// Como se ha terminado la operación reinicializamos variables
			Entrada = 0;
            CierreMercado = 0;
            Maximo = 0;
            Minimo = 0;
            operando=0;

		}

		tratados=i;

	}


// hemos rellenado la estructura de datos de los rangos para el horario definido.
// ahora vamos a rellenar el rango-trail con el dato accordado:
	//    1. rango-trail único y fijo
	//    2. rango trail fijo para cada año
	//    3. rango trail estimado y canculado 

char campo[20];
int vector_anyo[20];
float vector_trail[20];
int h=0, k=0, last_one=0;          // h= contador de elementos dentro del vector (años con trail)
						// k= contador de caracteres leidos en el campo

	if (RANGOTRAIL != 0)           // rango-trail único y fijo ***********************************************
	{
		for (int i = 0; strlen(datosrangos[i].fecha_EU)>0 ; ++i)
		{
			datosrangos[i].r_rangotrail = RANGOTRAIL;
			l=i;
		}
		last_one=l;
	}
	else if (RANGOTRAIL==0 && strcmp(TRAILFIJO, "c") !=0) // rango-trail fijo por cada año ****************************************
	{
		for (int j = 0; j < strlen(TRAILFIJO); ++j)     // primero cojo los años y sus trails y los pongo en vectores
		{
			if (TRAILFIJO[j]==':')
			{
				vector_anyo[h]= convCadenaEntero(campo);    // un vector para los años
				k=0; 
			}
			else if (TRAILFIJO[j]==',')
			{
				vector_trail[h]= ConvCadenaFloat(campo);    // otro vector para los correspondientes trails
				k=0;
				h++;
			}
			else
			{
				campo[k]=TRAILFIJO[j];
				campo[k+1]='\0';
				k++;
			}
		}
		if (k>1 && campo[k-1] != ',' && campo[k-1] != ':')
		{
			vector_trail[h]= ConvCadenaFloat(campo);
		}

		//for (int t = 0; t < h; ++t)
		//{
		//			printf("%d = %7.2f \n", vector_anyo[t],vector_trail[t]);
		//}


		h=0;
		while (vector_anyo[h]>0 && vector_trail[h]>0)				// parto del vector y recorro la estructura varias veces
		{
			for (int i = 0; strlen(datosrangos[i].fecha_EU)>0 ; ++i)   // pongo los trails en su correspondiente año 
			{															// dentro de la estructura de rangos
				cadenaano[0]=datosrangos[i].fecha_EU[6];			// compruebo la fecha y el años para poner su trail
				cadenaano[1]=datosrangos[i].fecha_EU[7];			
				cadenaano[2]=datosrangos[i].fecha_EU[8];
				cadenaano[3]=datosrangos[i].fecha_EU[9];
				cadenaano[4]='\0';
				numeroano=convCadenaEntero(cadenaano);
				if (numeroano==vector_anyo[h])
				{
					datosrangos[i].r_rangotrail = vector_trail[h];
				}
			}
			h++;
		}

	}
																//*******************************************************
	else if (RANGOTRAIL==0 && strcmp(TRAILFIJO, "c") ==0 && NUMSESIONES>0 && PORCREDUCCION>0 && strlen(ACTUALIZATRAIL)==1) //********
	{
		int pos_inicial=0, pos_ini_actual=0, pos_ini_origen=0, dia_semana_ini=0, primer_dia_lab=2, flag=0, idx=0, ano_num=0, mes_num=0;
		char ano_ini[10], ano_primer[5], mes_ini[10], mes_primer[6], mes_next[8];
		float mi_rango_trail=0.01, media=0.01;

		// inicializo el rangotrail de la estructura y calculo el indice del último elemento.

		for (int i = 0; strlen(datosrangos[i].fecha_EU)>0 ; ++i)
			{
				datosrangos[i].r_rangotrail = 0;
				l=i;
				//printf("%d: %s - %7.2f\n", i, datosrangos[i].fecha_EU, datosrangos[i].r_rangotrail );
			}
		last_one=l;
		//printf("%d\n", last_one);

	// calculo el trail estimado para cada periodo

			// Proceso :
			//	1. Me posiciono en el inicio y busco el punto de actualización(A,M,S,D) anterior. {a}
			//  2. Me voy tantas sesiones atras como numero de sesiones con que se estima. {b}
			//  3. Vuelvo al punto de actualización sumando rangos para estimar.
			//	4. Calculo el rango-trail (media de rangos * porcentaje y el resultado lo redondeo)
			//	5. Guardo el rango-trail en todas las sesiones hasta el nuevo punto de actualización.
			//	6. Vuelvo al punto 2 para seguir calculando el rango-trail para los siguientes puntos de actualizacion

		pos_inicial=posicionarseEnUnaFecha_EstructRangos(INIFECHA);

		if (strcmp(ACTUALIZATRAIL, "A")==0)
		{										// calculo el inicio laboral del año inicial
			printf("hasta aquí ****************************>>>>> %s\n", ACTUALIZATRAIL );
			
			snprintf(ano_primer,sizeof(ano_primer),"%c%c%c%c",INIFECHA[0],INIFECHA[1],INIFECHA[2],INIFECHA[3]);
			snprintf(ano_ini,sizeof(ano_ini),"%s%s%d",ano_primer,"010",primer_dia_lab);
			dia_semana_ini=calculaDiaSemana(ano_ini);
			while (dia_semana_ini==0 || dia_semana_ini==6)
			{
				primer_dia_lab++;
				snprintf(ano_ini,sizeof(ano_ini),"%s%s%d",ano_primer,"010",primer_dia_lab);
				dia_semana_ini=calculaDiaSemana(ano_ini);
			}
			printf("%s\n", ano_ini);
			pos_ini_actual=posicionarseEnUnaFecha_EstructRangos(ano_ini);      // me posiciono en el primer dia laboral

			pos_ini_origen = pos_ini_actual - NUMSESIONES;						// me posicione el punto origen para 
			
			if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
					printf("OJO: no hay suficiente historico para el cálculo del trail adecuado. Usamos el historico existente %d frente a %5.0f que no hay\n", pos_ini_actual-pos_ini_origen, NUMSESIONES);
				}
			media=0;
			for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
			{
				media=media+datosrangos[i].r_rango;
			}
			media=media/(pos_ini_actual - pos_ini_origen);
			mi_rango_trail=media*PORCREDUCCION/100;

			mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

			printf("%5d=%5d=%7.2f  >> last %d\n", pos_ini_actual, pos_ini_origen, NUMSESIONES, last_one);
			printf("%s => %7.2f => %7.2f\n",ano_primer, media, mi_rango_trail );

			// Actualizo el rango de la estructura para el año correspondiente
			
			flag = 0;
			ano_num= 0;
			idx=pos_ini_actual;
			while (flag==0 && idx<=last_one)
			{
				cadenaano[0]=datosrangos[idx].fecha_EU[6];			// compruebo la fecha y el año para poner su trail
				cadenaano[1]=datosrangos[idx].fecha_EU[7];			
				cadenaano[2]=datosrangos[idx].fecha_EU[8];
				cadenaano[3]=datosrangos[idx].fecha_EU[9];
				cadenaano[4]='\0';
				numeroano=convCadenaEntero(cadenaano);
				if (numeroano == convCadenaEntero(ano_primer)+ano_num)
				{
					datosrangos[idx].r_rangotrail=mi_rango_trail;
					idx++;
				}
				else
				{
					flag=1;
				}
			}

			/// bucle a seguir a partir del primer periodo hasta que llegue al final
			
			while (idx <= last_one)
			{
				ano_num++;                  					// añado uno más para control del siguiente año
				pos_ini_actual = idx;              // es el punto de partida para el siguiente año
				pos_ini_origen = pos_ini_actual - NUMSESIONES;						// me posicione el punto origen para 

				if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
				
				media=0;
				for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
				{
					media=media+datosrangos[i].r_rango;
				}
				media=media/(pos_ini_actual - pos_ini_origen);
				mi_rango_trail=media*PORCREDUCCION/100;

				mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

				// Actualizo el rango de la estructura para el año correspondiente

				flag = 0;
				idx=pos_ini_actual;
				while (flag==0 && idx<=last_one)
				{
					cadenaano[0]=datosrangos[idx].fecha_EU[6];			// compruebo la fecha y el año para poner su trail
					cadenaano[1]=datosrangos[idx].fecha_EU[7];			
					cadenaano[2]=datosrangos[idx].fecha_EU[8];
					cadenaano[3]=datosrangos[idx].fecha_EU[9];
					cadenaano[4]='\0';
					numeroano=convCadenaEntero(cadenaano);
					if (numeroano == convCadenaEntero(ano_primer)+ano_num)
					{
						datosrangos[idx].r_rangotrail=mi_rango_trail;
						idx++;
					}
					else
					{
						flag=1;
					}
				}
				printf("%5d=%5d=%7.2f  >> last %d\n", pos_ini_actual, pos_ini_origen, NUMSESIONES, last_one);
				printf("%d => %7.2f => %7.2f\n",convCadenaEntero(ano_primer)+ano_num, media, mi_rango_trail );
			}
		}


	// Vamos a por el caso de actualización mensual (M)

		if (strcmp(ACTUALIZATRAIL,"M")==0)
		{										// calculo el inicio laboral del primer mes a partir de la Fecha inicial
			primer_dia_lab=1;
			snprintf(mes_primer,sizeof(mes_primer),"%c%c%c%c%c%c",INIFECHA[0],INIFECHA[1],INIFECHA[2],INIFECHA[3],INIFECHA[4],INIFECHA[5]);
			snprintf(mes_ini,sizeof(mes_ini),"%s%s%d",mes_primer,"0",primer_dia_lab);
			dia_semana_ini=calculaDiaSemana(mes_ini);
			while (dia_semana_ini==0 || dia_semana_ini==6)
			{
				primer_dia_lab++;
				snprintf(mes_ini,sizeof(mes_ini),"%s%s%d",mes_primer,"0",primer_dia_lab);
				dia_semana_ini=calculaDiaSemana(mes_ini);
			}
			pos_ini_actual=posicionarseEnUnaFecha_EstructRangos(mes_ini);      // me posiciono en el primer dia laboral

			pos_ini_origen = pos_ini_actual - NUMSESIONES;
			if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
			media=0;												// me posicione el punto origen para 
			for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
			{
				media=media+datosrangos[i].r_rango;
			}
			media=media/(pos_ini_actual - pos_ini_origen);
			mi_rango_trail=media*PORCREDUCCION;

			mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

			// Actualizo el rango de la estructura para el año correspondiente
			
			flag = 0;
			mes_num = 0;
			idx=pos_ini_actual;
			while (flag==0 && idx<=last_one)
			{
				cade_names[0]=datosrangos[idx].fecha_EU[6];			// compruebo la fecha y el año para poner su trail
				cade_names[1]=datosrangos[idx].fecha_EU[7];			
				cade_names[2]=datosrangos[idx].fecha_EU[8];
				cade_names[3]=datosrangos[idx].fecha_EU[9];
				cade_names[4]=datosrangos[idx].fecha_EU[3];
				cade_names[5]=datosrangos[idx].fecha_EU[4];
				cade_names[6]='\0';
				numeromes=convCadenaEntero(cade_names);
				if (numeromes == convCadenaEntero(mes_primer)+ mes_num)
				{
					datosrangos[idx].r_rangotrail=mi_rango_trail;
					idx++;
				}
				else
				{
					flag=1;
					mes_next[0]=datosrangos[idx].fecha_EU[6];			// registro la fecha y el mes siguiente
					mes_next[1]=datosrangos[idx].fecha_EU[7];			
					mes_next[2]=datosrangos[idx].fecha_EU[8];
					mes_next[3]=datosrangos[idx].fecha_EU[9];
					mes_next[4]=datosrangos[idx].fecha_EU[3];
					mes_next[5]=datosrangos[idx].fecha_EU[4];
					mes_next[6]='\0';
				}
			}


			/// bucle a seguir a partir del primer periodo hasta que llegue al final
			
			while (idx <= last_one)
			{
				mes_num++;                 					// añado uno más para control del siguiente mes
				pos_ini_actual = idx;              			// es el punto de partida para el siguiente año
				pos_ini_origen = pos_ini_actual - NUMSESIONES;						// me posicione el punto origen para 
				if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
				media=0;
				for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
				{
					media=media+datosrangos[i].r_rango;
				}
				media=media/(pos_ini_actual-pos_ini_origen);
				mi_rango_trail=media*PORCREDUCCION;

				mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

				// Actualizo el rango de la estructura para el año correspondiente

				flag = 0;
				idx=pos_ini_actual;
				while (flag==0 && idx<=last_one)
				{
					cade_names[0]=datosrangos[idx].fecha_EU[6];			// compruebo la fecha y el año para poner su trail
					cade_names[1]=datosrangos[idx].fecha_EU[7];			
					cade_names[2]=datosrangos[idx].fecha_EU[8];
					cade_names[3]=datosrangos[idx].fecha_EU[9];
					cade_names[4]=datosrangos[idx].fecha_EU[3];
					cade_names[5]=datosrangos[idx].fecha_EU[4];
					cade_names[6]='\0';
					numeromes=convCadenaEntero(cade_names);
					if (numeromes == convCadenaEntero(mes_next))
					{
						datosrangos[idx].r_rangotrail=mi_rango_trail;
						idx++;
					}
					else
					{
						flag=1;
						mes_next[0]=datosrangos[idx].fecha_EU[6];			// registro la fecha y el mes siguiente
						mes_next[1]=datosrangos[idx].fecha_EU[7];			
						mes_next[2]=datosrangos[idx].fecha_EU[8];
						mes_next[3]=datosrangos[idx].fecha_EU[9];
						mes_next[4]=datosrangos[idx].fecha_EU[3];
						mes_next[5]=datosrangos[idx].fecha_EU[4];
						mes_next[6]='\0';
					}

				}
			}
		}

		if (strcmp(ACTUALIZATRAIL,"S")==0)
		{
			pos_ini_actual=posicionarseEnUnaFecha_EstructRangos(INIFECHA);      // me posiciono en el primer dia
			dia_semana_ini=calculaDiaSemana(INIFECHA);
			while (dia_semana_ini!=1)
			{
				pos_ini_actual++;
				cadenafecha[0]=datosrangos[pos_ini_actual].fecha_EU[6];	// compruebo la fecha que es, para ver cuando
				cadenafecha[1]=datosrangos[pos_ini_actual].fecha_EU[7];	// cambio de fecha
				cadenafecha[2]=datosrangos[pos_ini_actual].fecha_EU[8];
				cadenafecha[3]=datosrangos[pos_ini_actual].fecha_EU[9];
				cadenafecha[4]=datosrangos[pos_ini_actual].fecha_EU[3];
				cadenafecha[5]=datosrangos[pos_ini_actual].fecha_EU[4];
				cadenafecha[6]=datosrangos[pos_ini_actual].fecha_EU[0];
				cadenafecha[7]=datosrangos[pos_ini_actual].fecha_EU[1];
				cadenafecha[8]='\0';
				dia_semana_ini=calculaDiaSemana(cadenafecha);
			}
			// termino en la posicion del primer dia lunes despues de la fecha inicial =>> pos_ini_actual

			pos_ini_origen = pos_ini_actual - NUMSESIONES;
			if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
			media=0;												// me posicione el punto origen para 
			for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
			{
				media=media+datosrangos[i].r_rango;
			}
			media=media/(pos_ini_actual-pos_ini_origen);
			mi_rango_trail=media*PORCREDUCCION;

			mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

			// Actualizo el rango de la estructura para la semana correspondiente
			
			flag = 0;
			mes_num = 0;
			idx=pos_ini_actual;
			while (flag==0 && idx<=last_one)
			{
				cadenafecha[0]=datosrangos[idx].fecha_EU[6];	// compruebo la fecha y el año para poner su trail
				cadenafecha[1]=datosrangos[idx].fecha_EU[7];			
				cadenafecha[2]=datosrangos[idx].fecha_EU[8];
				cadenafecha[3]=datosrangos[idx].fecha_EU[9];
				cadenafecha[4]=datosrangos[idx].fecha_EU[3];
				cadenafecha[5]=datosrangos[idx].fecha_EU[4];
				cadenafecha[6]=datosrangos[idx].fecha_EU[0];
				cadenafecha[7]=datosrangos[idx].fecha_EU[1];
				cadenafecha[8]='\0';
				dia_semana_ini=calculaDiaSemana(cadenafecha);
				if (dia_semana_ini >= 1 && dia_semana_ini <= 5 && (idx - pos_ini_actual) < 7)
				{
					datosrangos[idx].r_rangotrail=mi_rango_trail;
					idx++;
				}
				else
				{
					flag=1;
				}
			}


			/// bucle a seguir a partir del primer periodo hasta que llegue al final
			
			while (idx <= last_one)
			{
                  					// añado uno más para control del siguiente mes
				pos_ini_actual = idx;              			// es el punto de partida para el siguiente año
				pos_ini_origen = pos_ini_actual - NUMSESIONES;						// me posicione el punto origen para 
				if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
				media=0;
				for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
				{
					media=media+datosrangos[i].r_rango;
				}
				media=media/(pos_ini_actual-pos_ini_origen);
				mi_rango_trail=media*PORCREDUCCION;

				mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

				// Actualizo el rango de la estructura para el año correspondiente

				flag = 0;
				idx=pos_ini_actual;
				while (flag==0 && idx<=last_one)
				{
					cadenafecha[0]=datosrangos[idx].fecha_EU[6];			// compruebo la fecha y el año para poner su trail
					cadenafecha[1]=datosrangos[idx].fecha_EU[7];			
					cadenafecha[2]=datosrangos[idx].fecha_EU[8];
					cadenafecha[3]=datosrangos[idx].fecha_EU[9];
					cadenafecha[4]=datosrangos[idx].fecha_EU[3];
					cadenafecha[5]=datosrangos[idx].fecha_EU[4];
					cadenafecha[6]=datosrangos[idx].fecha_EU[0];
					cadenafecha[7]=datosrangos[idx].fecha_EU[1];
					cadenafecha[8]='\0';
					dia_semana_ini=calculaDiaSemana(cadenafecha);
					if (dia_semana_ini >= 1 && dia_semana_ini <= 5 && (idx - pos_ini_actual) < 7)
					{
						datosrangos[idx].r_rangotrail=mi_rango_trail;
						idx++;
					}
					else
					{
						flag=1;
					}

				}
			}


		}

		if (strcmp(ACTUALIZATRAIL,"D")==0)
		{
			pos_ini_actual=posicionarseEnUnaFecha_EstructRangos(INIFECHA);      // me posiciono en el primer dia
			
			pos_ini_origen = pos_ini_actual - NUMSESIONES;
			if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
			media=0;												// me posicione el punto origen para 
			for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
			{
				media=media+datosrangos[i].r_rango;
			}
			media=media/(pos_ini_actual-pos_ini_origen);
			mi_rango_trail=media*PORCREDUCCION;

			mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

			// Actualizo el rango de la estructura para el día correspondiente
			
			flag = 0;
			mes_num = 0;
			idx=pos_ini_actual;
			while (flag==0 && idx<=last_one)
			{
				datosrangos[idx].r_rangotrail=mi_rango_trail;
				idx++;
				flag=1;
			}

			/// bucle a seguir a partir del primer periodo hasta que llegue al final
			
			while (idx <= last_one)
			{
                  					// añado uno más para control del siguiente mes
				pos_ini_actual = idx;              			// es el punto de partida para el siguiente año
				pos_ini_origen = pos_ini_actual - NUMSESIONES;						// me posicione el punto origen para 
				if (pos_ini_origen<0) 												// si no hay sesiones suficientes empezaré desde el principio
				{
					pos_ini_origen=0;
				}
				media=0;
				for (int i = pos_ini_origen; i < pos_ini_actual ; ++i)              // calcular el rango
				{
					media=media+datosrangos[i].r_rango;
				}
				media=media/(pos_ini_actual-pos_ini_origen);
				mi_rango_trail=media*PORCREDUCCION;

				mi_rango_trail = redondeoSegunContrato(mi_rango_trail);          // redondeo a mi entero (REVISAR)

				// Actualizo el rango de la estructura para el día correspondiente

				flag = 0;
				idx=pos_ini_actual;
				while (flag==0 && idx<=last_one)
				{
					datosrangos[idx].r_rangotrail=mi_rango_trail;
					idx++;
					flag=1;
				}
			}

		}

	}

	if (tratados>0)
		{ return 1;}
	else
		{ return 0;}

}

int escribeFicheroResultadoRangos(char mifichero[])
{
	int retorno, es=0;
	FILE *U1;
	U1=fopen(mifichero, "w");
	if (U1 !=NULL)
		{	
			fprintf(U1,"%s;%s;%s;%s;%s;%s;%s;%s;%s","Fecha","Hora ini","Hora fin","Apertura", "Cierre", "Maximo", "Minimo", "Rango", "Trail");
			while(strlen(datosrangos[es].fecha_EU)>8)
			{
				fprintf(U1,"%s;%s;%s;",datosrangos[es].fecha_EU,datosrangos[es].hora_ini,datosrangos[es].hora_fin);
				fprintf(U1,"%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f\n",datosrangos[es].r_apertura,datosrangos[es].r_cierre,datosrangos[es].r_maximo,datosrangos[es].r_minimo,datosrangos[es].r_rango,datosrangos[es].r_rangotrail);
				es++;
			}
			retorno=1;
		}
	else
		{	retorno=0;}
	fclose(U1);
	return retorno;
}


int posicionarseEnUnaFecha_EstructRangos(char mifecha[])
{

	char cadenaano[5], cadenames[3], cadenadia[3], cadenahora[3], cadenaminu[3], Fecha_EU[11];
	int FechaAnt=0, Fecha=0, miFecha=0, numeroano=0, numeromes=0, numerodia=0, pos=0;

	miFecha=convCadenaEntero(mifecha);

	for (int i = 0; strlen(datosrangos[i].fecha_EU)>0; ++i)
	{
		cadenaano[0]=datosrangos[i].fecha_EU[6];			// compruebo la fecha que es, para ver cuando
		cadenaano[1]=datosrangos[i].fecha_EU[7];			// cambio de fecha
		cadenaano[2]=datosrangos[i].fecha_EU[8];
		cadenaano[3]=datosrangos[i].fecha_EU[9];
		cadenaano[4]='\0';
		cadenames[0]=datosrangos[i].fecha_EU[3];
		cadenames[1]=datosrangos[i].fecha_EU[4];
		cadenames[2]='\0';
		cadenadia[0]=datosrangos[i].fecha_EU[0];
		cadenadia[1]=datosrangos[i].fecha_EU[1];
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
	}

	return pos;

}

int posicionarseEnUnaFecha_EstructOperativa(char mifecha[])
{

	char cadenaano[5], cadenames[3], cadenadia[3], cadenahora[3], cadenaminu[3], Fecha_EU[11];
	int FechaAnt=0, Fecha=0, miFecha=0, numeroano=0, numeromes=0, numerodia=0, pos=0;

	miFecha=convCadenaEntero(mifecha);

	for (int i = 0; strlen(datosoperativa[i].fecha_EU)>0; ++i)
	{
		cadenaano[0]=datosoperativa[i].fecha_EU[6];			// compruebo la fecha que es, para ver cuando
		cadenaano[1]=datosoperativa[i].fecha_EU[7];			// cambio de fecha
		cadenaano[2]=datosoperativa[i].fecha_EU[8];
		cadenaano[3]=datosoperativa[i].fecha_EU[9];
		cadenaano[4]='\0';
		cadenames[0]=datosoperativa[i].fecha_EU[3];
		cadenames[1]=datosoperativa[i].fecha_EU[4];
		cadenames[2]='\0';
		cadenadia[0]=datosoperativa[i].fecha_EU[0];
		cadenadia[1]=datosoperativa[i].fecha_EU[1];
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
	}

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


int OperacionTecnicaEX(char ini_fecha[], char fin_fecha[])

{

// Definicion de variables a considerar

	char Fecha[11];
	float Apertura, CierreLargos, CierreCortos;
	char Largos[2] = "L";
	char Cortos[2] = "C";
	char Operacion[2] = "L";   // Tipo de operación que ejecuto al principio del dia y según como ha terminado el día anterior
	char OperacionAnt[2] = "C";
	float Limite = 0.0;             // Es el valor alcanzado en contra-tendencia y que nos hace cambiar (L/C)
	float LimiteAnt = 0.0;
	float Objetivo = 0.0;    // Es el valor alcanzado en tendencia y que nos invita a cambiar (L/C) una vez que se da la vuelta (20%)
	float ObjetivoAnt = 0.0;
	float NumRngLimit = NUMEJESLIM;     // ************ ' Es la cantidad de Rangos utilizados como limite para darse la vuelta (L/C)
	float Trail = 15.0;          // Rango utilizado para el calculo de las operaciones (cada año/mes/semana/dia puede variar)
	float TrailAnt = 0.0;          // El Rango puede variar con los años
	float TamanoEje = 15.0;       //************   ' Es el tamaño de cada eje del gráfico y que usaremos para contar el objetivo
	float TamanoEjeAnt = 0.0;
	int NumEjes = NUMEJESOBJ;           //************   ' Es la cantidad de ejes a considerar como objetivo para darse la vuelta (L/C)
	int NumEjesAnt = 0;
	int ResultadoAcumulado = 0;   // Variable que irá acumulando el resultado de operar según la estrategia
	float Tic;
	float CantidadContratos = 0;    // Contratos con los que entramos en mercado.
	float Multiplicador = 0;

// Bucle principal del programa que va analizando día a día la operación y genera su gráfico operativo y de resultados

// Iniciamos la variables para comenzar

	float ResultadoLargos = 0.0, ResultadoCortos = 0.0;
	float AcumulaLargos = 0.0, AcumulaCortos =0.0;      // Variable donde acumulo restultado a largos  y cortos (la curva operativa)
	float LimiteSig = 0.0 - NumRngLimit * TamanoEje; // Empiezo con largos desde 0 y el limite estará en zona negativa
	float ObjetivoSig = NumEjes * TamanoEje;         // Empiezo con largos desde 0 y el limite estará en zona positiva
	char OperacionSig[2] = "L";                      // Empiezo con largos                           
	int ObjetivoAlcanzado = 0;                       // Variable que le indica si ya he alcanzado el objetivo de la tendencia
	int SigoIgual = 1;                               // Variable que me indica si en el analisis se ha cumplido alguna condición.

	int cont=0, sigue=1, encontrado=0;            // Contador de registros a lo largo de la estructura de datos
	int pos_inicial=0, pos_final=0;				// posición inicial en la estructura a partir de la cual calculamos rendimientos

	Tic=TICMOV;

	// Contratos iniciales

	CantidadContratos = CONTRATOSINICIALES;
	Multiplicador = MULTIPLICADOR;

	// Buscamos el comienzo de calculo para fecha inicial seleccionada. Nos posicionamos en esa fecha

	pos_inicial=posicionarseEnUnaFecha_EstructOperativa(INIFECHA);
	cont=pos_inicial;

	// Buscamos el final del calculo con la fecha final seleccionada, para salir del bucle

	pos_final=posicionarseEnUnaFecha_EstructOperativa(FINFECHA);
	if (pos_final==0)                              // busco la ultima posición de la estructura de datos
	{
		for (int e = 0; (strlen(datosrangos[e].fecha_EU) > 8); ++e)
			{
				encontrado=e;
			}
		pos_final=encontrado;
	}

	printf("%s - %d : %s - %d : \n", INIFECHA, pos_inicial, FINFECHA, pos_final );


	// Asignamos el tamaño de eje al Trail. De momento porque no hay forma automática de redondeo

	TamanoEje=Trail;

	while (sigue==1 & strncmp(datosoperativa[cont].fecha_EU, "\0", 1) !=0 & cont <= pos_final)   // bucle
	{
	//Busco el rango-trail en la estructura de rangos.
		encontrado=0;
		for (int e = 0; (strlen(datosrangos[e].fecha_EU) > 8 && encontrado==0); ++e)
			{
				if (strcmp(datosrangos[e].fecha_EU,datosoperativa[cont].fecha_EU)==0)
				{
					Trail=datosrangos[e].r_rangotrail;
					encontrado=1;
					TamanoEje=Trail;
					if (Limite==0.0 & Objetivo==0.0)               // Solo para el comienzo de calculo 
					{
						LimiteSig = 0.0 - NumRngLimit * TamanoEje;
						ObjetivoSig = NumEjes * TamanoEje;
					}
				}
			}
    // Recojo la información de la estructura de datos
    // Cargo los datos de apertura, cierre largos, cierre de cortos y algo mas

    	strcpy(Fecha, datosoperativa[cont].fecha_EU);
    	Apertura=datosoperativa[cont].apertura;
    	CierreLargos=datosoperativa[cont].cierrelargos;
    	CierreCortos=datosoperativa[cont].cierrecortos;

    // Asigno la operativa a realizar, el limite y el objetivo

    	strcpy(Operacion, OperacionSig);
    	Limite = LimiteSig;
    	Objetivo = ObjetivoSig;

    // Calculo el indicador acumulado de largos
    // Justo despues de operar en el día de la fecha. Toca analizar

    	ResultadoLargos = CierreLargos - Apertura - Tic;
    	ResultadoCortos = Apertura - CierreCortos - Tic;
    	AcumulaLargos = AcumulaLargos + ResultadoLargos ;
    	AcumulaCortos = AcumulaCortos + ResultadoCortos ;

    	if (strcmp(Operacion, Largos)==0)
    	{
        	ResultadoAcumulado = ResultadoAcumulado + ResultadoLargos * CantidadContratos * Multiplicador;
    	}
    	if (strcmp(Operacion, Cortos)==0)
    	{
        	ResultadoAcumulado = ResultadoAcumulado + ResultadoCortos * CantidadContratos * Multiplicador;   
    	}

    	SigoIgual=1;

    	if (strcmp(Operacion, Largos)==0)                         // Si estamos LARGOS
    	{
        	if (AcumulaLargos <= Limite & SigoIgual == 1)        // Alcanzamos o superamos el limite y cambiamos a cortos
        	{
            	strcpy(OperacionAnt, Largos);
            	strcpy(OperacionSig, Cortos);
            
            	LimiteAnt = Limite;
            	LimiteSig = AcumulaLargos + (NumRngLimit * TamanoEje);
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = AcumulaLargos - (NumEjes * TamanoEje);
            
            	SigoIgual = 0;
        	}
        
        
        	if (ObjetivoAlcanzado == 1 & ResultadoLargos <= ((0 - 1) * Trail * 0.2) & SigoIgual == 1) // Objetivo alcanzado con retroceso del 20% y cambiamos
        	{
            	strcpy(OperacionAnt, Largos);
            	strcpy(OperacionSig, Cortos);
            
            	LimiteAnt = Limite;
            	LimiteSig = AcumulaLargos + (NumRngLimit * TamanoEje);
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = AcumulaLargos - (NumEjes * TamanoEje);
            
            	ObjetivoAlcanzado = 0;
            	SigoIgual = 0;
        	}
        
        	if (AcumulaLargos > Objetivo & ObjetivoAlcanzado == 0 & SigoIgual == 1)             // Alcanzamos el objetivo pero esperamos al retroceso
        	{
            	ObjetivoAlcanzado = 1;
            
            	strcpy(OperacionAnt, Operacion);
            	strcpy(OperacionSig, OperacionAnt);
            
            	LimiteAnt = Limite;
            	LimiteSig = LimiteAnt;
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = ObjetivoAnt;
            
            	SigoIgual = 0;
        	}
        
    	}

    	if (strcmp(Operacion, Cortos)==0 )                                                  // Si estamos CORTOS
    	{
        	if (AcumulaLargos >= Limite & SigoIgual == 1)             // Alcanzamos el limite y cambiamos
        	{
            	strcpy(OperacionAnt, Cortos);
            	strcpy(OperacionSig, Largos);
            
            	LimiteAnt = Limite;
            	LimiteSig = AcumulaLargos - (NumRngLimit * TamanoEje);
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = AcumulaLargos + (NumEjes * TamanoEje);
            
            	SigoIgual = 0;
        	}
        
        	if (ObjetivoAlcanzado == 1 & ResultadoLargos >= ((1) * Trail * 0.2) & SigoIgual == 1)  // Objetivo alcanzado con retroceso del 20% y cambiamos
        	{
            	strcpy(OperacionAnt, Cortos);
            	strcpy(OperacionSig, Largos);
            
            	LimiteAnt = Limite;
            	LimiteSig = AcumulaLargos - (NumRngLimit * TamanoEje);
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = AcumulaLargos + (NumEjes * TamanoEje);
            
            	ObjetivoAlcanzado = 0;
            	SigoIgual = 0;
        	}
        
        	if (AcumulaLargos < Objetivo & ObjetivoAlcanzado == 0 & SigoIgual == 1)           // Alcanzamos el objetivo y esperamos a el retroceso
        	{
            	ObjetivoAlcanzado = 1;
            
            	strcpy(OperacionAnt, Operacion);
            	strcpy(OperacionSig, OperacionAnt);
            
            	LimiteAnt = Limite;
            	LimiteSig = Limite;
            
            	ObjetivoAnt = Objetivo;
            	ObjetivoSig = Objetivo;
            
            	SigoIgual = 0;
        	}
        
    	}

    	if (SigoIgual == 1)   // No pasa nada, no se se alcanza ni limite ni objetivo, sigo con la operación de la misma forma
    	{
        	strcpy(OperacionAnt, Operacion);
        	strcpy(OperacionSig, OperacionAnt);
        
        	LimiteAnt = Limite;
        	LimiteSig = LimiteAnt;

        	ObjetivoAnt = Objetivo;
        	ObjetivoSig = ObjetivoAnt;
    	}

                                                     // almaceno el resultado diario
    	datosoperativa[cont].acumulalargos=AcumulaLargos;
    	datosoperativa[cont].acumulacortos=AcumulaCortos;
    	datosoperativa[cont].objetivo=ObjetivoAnt;
    	datosoperativa[cont].limite=LimiteAnt;
    	strcpy(datosoperativa[cont].operacion, OperacionAnt);
    	datosoperativa[cont].cantidadcontratos = CantidadContratos;
    	datosoperativa[cont].multiplicador = Multiplicador;
    	datosoperativa[cont].resultadoacumulado=ResultadoAcumulado;

    	cont = cont + 1;

	}  

return  1;

// Fin del código
}

int escribeResultadoTecnicaEX(char mifichero[])
{
    int retorno, es=0;
    FILE *U1;
    U1=fopen(mifichero, "w");
    if (U1 !=NULL)
        {   
            fprintf(U1,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n","Fecha","Hora Ini","Hora Fin","Apertura","CierreLargos","CierreCortos","CierreMercado","Maximo","Minimo","Trail","AltaVolatilidad","AcumulaLargos","AcumulaCortos","Objetivo","Limite","Operacion","Cant_Contratos","Multiplicador","ResultadoAcumulado");
            while(strlen(datosoperativa[es].fecha_EU)>8)
            {
                fprintf(U1,"%s;%s;%s;",datosoperativa[es].fecha_EU,datosoperativa[es].hora_ini,datosoperativa[es].hora_fin);
                fprintf(U1,"%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;%8.2f;",datosoperativa[es].apertura,datosoperativa[es].cierrelargos,datosoperativa[es].cierrecortos,datosoperativa[es].cierremercado,datosoperativa[es].maximo,datosoperativa[es].minimo,datosoperativa[es].rangotrail);
                fprintf(U1,"%3d;%8.2f;%8.2f;%8.2f;%8.2f;%s;%3d;%5d;%8.2f\n",datosoperativa[es].altavolatilidad,datosoperativa[es].acumulalargos,datosoperativa[es].acumulacortos,datosoperativa[es].objetivo,datosoperativa[es].limite,datosoperativa[es].operacion,datosoperativa[es].cantidadcontratos,datosoperativa[es].multiplicador,datosoperativa[es].resultadoacumulado);
                es++;

            }
            retorno=1;
        }
    else
        {   retorno=0;}
    fclose(U1);
    return retorno;
}

int redondeoSegunContrato(float mi_numerito)
{
	float mi_numero_redondeado = 0.0;
	// funcion de redondeo durante el proceso de estimación del rango.
	// SE = redondeo a la unidad siguiente (de 24.25 paso a 25.00)
	// NQ = igual que el SE
	// CL = redondeo a la decima siguiente (de 1.56  paso a 1.60 )

	if (strcmp(MICONTRATO,"ES")==0 || strcmp(MICONTRATO,"NQ")==0 || strcmp(MICONTRATO,"YM")==0)
	{
		mi_numero_redondeado=ceil(mi_numerito); // redondeo al alza
	}
	if (strcmp(MICONTRATO,"CL")==0)
	{
		mi_numero_redondeado=ceil(mi_numerito*10)/10;     // redondeo al siguiente decima
	}

	return mi_numero_redondeado;

}

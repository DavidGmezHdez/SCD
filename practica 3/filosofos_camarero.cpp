// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,
   num_procesos  = 2*num_filosofos+1 ,
   id_camarero = 10;
   
   /*
   Equivalencias de numeros
      0-> Soltar tenedor
      1-> Coger tenedor
      2-> Sentarse
      3-> Levantarse
   */

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_procesos, //id. tenedor izq.
      id_ten_der = (id+num_procesos-1) % num_procesos, //id. tenedor der.
      valor=1;
   MPI_Status estado;

  while ( true )
  {
   if(id==0)
   // ... solicitar sentarse
   cout<<"Filosofo "<<id<<" solicita sentarse"<<endl;
   MPI_Ssend(&valor,1,MPI_INT,id_camarero,2,MPI_COMM_WORLD);

   // ... esperar asiento
   MPI_Recv(&valor,1,MPI_INT,id_camarero,2,MPI_COMM_WORLD,&estado);
    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor izquierdo
    MPI_Ssend(&valor,1,MPI_INT,id_ten_izq,1,MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor derecho
    MPI_Ssend(&valor,1,MPI_INT,id_ten_der,1,MPI_COMM_WORLD);
      cout <<"Filósofo " <<id << " solicita ten. der." <<id_ten_izq <<endl;
    // ... solicitar tenedor derecho
    MPI_Ssend(&valor,1,MPI_INT,id_ten_der,1,MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" solicita ten. izq." <<id_ten_der <<endl;
    // ... solicitar tenedor izquierdo
    MPI_Ssend(&valor,1,MPI_INT,id_ten_izq,1,MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo w
    MPI_Ssend(&valor,1,MPI_INT,id_ten_izq,0,MPI_COMM_WORLD);

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho 
    MPI_Ssend(&valor,1,MPI_INT,id_ten_der,0,MPI_COMM_WORLD);

    // ... levantarse
    MPI_Ssend(&valor,1,MPI_INT,id_camarero,2,MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar" << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo 
     MPI_Recv(&valor,1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&estado);
     // ...... guardar en 'id_filosofo' el id. del emisor 
     id_filosofo=estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo'
     MPI_Recv(&valor,1,MPI_INT,id_filosofo,0,MPI_COMM_WORLD,&estado);
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

void funcion_camarero(){
   int valor,id_filosofo;
   int n=0;
   MPI_Status estado;

   while (true)
   {
      estado.MPI_TAG=0;

      if(n<4){
         MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&estado);
      }
      else{
         MPI_Probe(MPI_ANY_SOURCE,3,MPI_COMM_WORLD,&estado);
      }

      if(estado.MPI_TAG==2){
         id_filosofo=estado.MPI_SOURCE;
         MPI_Recv(&valor,1,MPI_INT,id_filosofo,2,MPI_COMM_WORLD,&estado);
         n++;
         MPI_Ssend(&valor,1,MPI_INT,id_filosofo,2,MPI_COMM_WORLD);
         cout<<"Camarero: Filosofo "<<id_filosofo<<" se ha sentado. Numero de filósofos sentados: "<<n<<endl;
      }
      else if(estado.MPI_TAG==3){
         id_filosofo=estado.MPI_SOURCE;
         MPI_Recv(&valor,1,MPI_INT,id_filosofo,3,MPI_COMM_WORLD,&estado);
         n--;
         cout<<"Camarero: Filosofo "<<id_filosofo<<" se ha levantado. Numero de filósofos sentados: "<<n<<endl;
      }
   }
}


// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == id_camarero)
         funcion_camarero();
      else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}



// ---------------------------------------------------------------------

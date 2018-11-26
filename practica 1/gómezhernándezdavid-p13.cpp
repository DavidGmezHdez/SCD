#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

mutex Mutex;
const int MAX=3;
Semaphore ingredientes[MAX]={0,0,0};
Semaphore mostrador_vacio=1;


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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  int ingr;

    chrono::milliseconds duracion_producir( aleatorio<20,200>() );

    this_thread::sleep_for(duracion_producir);
   while (true){
      ingr = aleatorio<0,2>();
      sem_wait(mostrador_vacio);
       Mutex.lock();
      cout<<"Puesto ingrediente: "<<ingr<<endl;
      Mutex.unlock();
      sem_signal(ingredientes[ingr]);
    }



}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

    Mutex.lock();

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,2000>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

    Mutex.unlock();

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
  int contador=0;
   while( true )
   {
      sem_wait(ingredientes[num_fumador]);
      Mutex.lock();
      cout<<"Retirado el ingredientes: "<<num_fumador<<endl;
      Mutex.unlock();
      sem_signal(mostrador_vacio);

      if(contador!= 4){
        fumar(num_fumador);
        contador+=1;
      }
      else
      {
        Mutex.lock();
        cout<<"fumador nº"<<num_fumador<<" Quien lo mata?";
        contador=0;
        Mutex.unlock();
      }
      

   }
  }


//----------------------------------------------------------------------

int main()
{
  cout << "--------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

   thread hebra_estanquero( funcion_hebra_estanquero);
   thread hebra_fumador[MAX];
   for (int i = 0; i < MAX; i++)
   {
     hebra_fumador[i]=thread(funcion_hebra_fumador,i);
   }
   for (int i = 0; i < MAX; i++)
   {
     hebra_fumador[i].join();
   }
   hebra_estanquero.join();
   
}

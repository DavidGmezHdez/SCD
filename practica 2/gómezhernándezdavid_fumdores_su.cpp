// -----------------------------------------------------------------------------
//David Gómez Hernández 2ºB
// Sistemas concurrentes y Distribuidos.
//
// archivo: fumadores_su.cpp
// Ejemplo de un monitor 'Fumadores' parcial, con semántica SU
//
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int num_fumadores=7;
mutex Mutex;


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// *****************************************************************************
// clase para monitor Estanco,  semántica SU

class Estanco : public HoareMonitor
{
   private:
   int      num_ingrediente;
   CondVar  mostrador_vacio,
            ingrediente_disponible[num_fumadores];
   public:
   Estanco() ; // constructor
   //funciones
   void obtenerIngredientes(int ingrediente);
   void ponerIngrediente(int ingrediente);
   void esperarRecogidaIngrediente();
} ;
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   num_ingrediente = -1;
   mostrador_vacio=newCondVar();
   for(unsigned i=0;i<num_fumadores;i++)
      ingrediente_disponible[i]=newCondVar();   
}
// -----------------------------------------------------------------------------
void Estanco::obtenerIngredientes(int ingrediente)
{
   if(num_ingrediente!=ingrediente)
      ingrediente_disponible[ingrediente].wait();

   num_ingrediente=-1;

   Mutex.lock();
   cout<<"           Ingrediente "<<ingrediente<<" retirado"<<endl;
   Mutex.unlock();
   mostrador_vacio.signal();
}

void Fumar(int num_fumador)
{  
   chrono::milliseconds fumar(aleatorio<200,600>());
   Mutex.lock();
   cout<<"           Fumador "<<num_fumador<<" comienza a fumar"<<endl;
   Mutex.unlock();
   this_thread::sleep_for(fumar);
   Mutex.lock();
   cout<<"           Fumador "<<num_fumador<<" termina de fumar"<<endl;
   Mutex.unlock();
}

int ProducirIngrediente()
{  
   chrono::milliseconds poner(aleatorio<200,600>());
   this_thread::sleep_for(poner);
   int ingrediente = aleatorio<0,num_fumadores-1>();
   return ingrediente;
}

void Estanco::ponerIngrediente(int ingrediente)
{
   Mutex.lock();
   cout<<"Poniendo ingrediente "<<ingrediente<<endl;
   Mutex.unlock();

   ingrediente_disponible[ingrediente].signal();
}

void Estanco::esperarRecogidaIngrediente(){
   if (num_ingrediente!=-1)
      mostrador_vacio.wait();
}


// -----------------------------------------------------------------------------


void funcion_hebra_fumador( MRef<Estanco> monitor, int ingrediente )
{
   while( true )
   {
      monitor->obtenerIngredientes(ingrediente);
      Fumar(ingrediente);;
   }
}

void funcion_hebra_estanquero(MRef<Estanco> monitor)
{
   while( true )
   {
      int ingrediente=ProducirIngrediente();
      monitor->ponerIngrediente(ingrediente);
      monitor->esperarRecogidaIngrediente();
   }
}

// *****************************************************************************

int main()
{
   cout <<  "Estanco: inicio simulación." << endl ;

   // crear monitor
   MRef<Estanco> monitor = Create<Estanco>();

   // crear y lanzar hebras
   thread hebra_estanquero(funcion_hebra_estanquero,monitor);;
   thread hebras_fumador[num_fumadores];
   
   for( unsigned i = 0 ; i < num_fumadores ; i++ )
      hebras_fumador[i] = thread( funcion_hebra_fumador, monitor, i );

   // esperar a que terminen las hebras (no pasa nunca)
   for( unsigned i = 0 ; i < num_fumadores ; i++ )
      hebras_fumador[i].join();

   hebra_estanquero.join();
}

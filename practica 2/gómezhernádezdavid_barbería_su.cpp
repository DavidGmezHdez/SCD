// -----------------------------------------------------------------------------
//David Gómez Hernández 2ºB
// Sistemas concurrentes y Distribuidos.
//
// archivo: barberia_su.cpp
// Ejemplo de un monitor 'Barberia' parcial, con semántica SU
//
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

mutex Mutex;


template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// *****************************************************************************
// clase para monitor Barbería,  semántica SU

class Barberia : public HoareMonitor
{
   private:
   int      cliente;
   CondVar  sala_espera,
            ocupado,
            cortando;

   public:
   Barberia() ; // constructor
   //funciones
   void CortarPelo(int cliente);
   void siguienteCliente();
   void finCliente();
} ;
// -----------------------------------------------------------------------------

Barberia::Barberia()
{
   cliente = 0;
   sala_espera=newCondVar();
   ocupado=newCondVar();
   cortando=newCondVar();
}
// -----------------------------------------------------------------------------
void Barberia::CortarPelo(int cliente)
{
   Mutex.lock();
   cout<<"El cliente "<<cliente<<" llega a la barberia"<<endl;
   Mutex.unlock();

   if(sala_espera.empty()){
      ocupado.wait();
   }
   else{
      sala_espera.signal();
   }

   Mutex.lock();
   cout<<"El cliente "<<cliente<<" se está cortando el pelo"<<endl;
   Mutex.unlock();

   cortando.wait();
}

void EsperarFueraBarberia(int cliente)
{
   chrono::milliseconds espera(aleatorio<200,600>());
   Mutex.lock();
   cout<<"El cliente "<<cliente<<" sale de la barberia"<<endl;
   Mutex.unlock();
   this_thread::sleep_for(espera);
}

void Barberia::siguienteCliente()
{
   if(ocupado.empty()){
      sala_espera.wait();
   }
   else{
      ocupado.signal();
   }
}

void CortarPeloCliente()
{
   chrono::milliseconds cortando(aleatorio<30,150>());
   Mutex.lock();
   cout<<"El barbero corta el pelo al cliente"<<endl;
   Mutex.unlock();
   this_thread::sleep_for(cortando);
}

void Barberia::finCliente()
{
   cortando.signal();
}

// -----------------------------------------------------------------------------


void funcion_hebra_cliente( MRef<Barberia> monitor, int num_cliente )
{
   while( true )
   {
      monitor->CortarPelo(num_cliente);
      EsperarFueraBarberia(num_cliente);
   }
}

void funcion_hebra_barbero(MRef<Barberia> monitor)
{
   while( true )
   {
      monitor->siguienteCliente();
      CortarPeloCliente();
      monitor->finCliente();
   }
}

// *****************************************************************************

int main()
{
   cout <<  "Barberia: inicio simulación." << endl ;

   // declarar el número total de hebras
   const int num_clientes=7;

   // crear monitor
   MRef<Barberia> monitor = Create<Barberia>();

   // crear y lanzar hebras
   thread hebra_barbero(funcion_hebra_barbero,monitor);;
   thread hebras_cliente[num_clientes];
   
   for( unsigned i = 0 ; i < num_clientes ; i++ )
      hebras_cliente[i] = thread( funcion_hebra_cliente, monitor, i );

   // esperar a que terminen las hebras (no pasa nunca)
   for( unsigned i = 0 ; i < num_clientes ; i++ )
      hebras_cliente[i].join();

   hebra_barbero.join();
}

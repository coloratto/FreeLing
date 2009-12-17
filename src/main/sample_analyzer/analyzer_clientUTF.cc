//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


//------------------------------------------------------------------//
//
//                    IMPORTANT NOTICE
//
//  This file contains a simple main program to illustrate 
//  usage of FreeLing analyzers library.
//
//  This sample main program may be used straightforwardly as 
//  a basic front-end for the analyzers (e.g. to analyze corpora)
//
//  Neverthless, if you want embed the FreeLing libraries inside
//  a larger application, or you want to deal with other 
//  input/output formats (e.g. XML), the efficient and elegant 
//  way to do so is consider this file as a mere example, and call 
//  the library from your your own main code.
//
//------------------------------------------------------------------//


#include <fstream>
#include <iostream>
#include <string>
#include <pthread.h>

using namespace std;



static char UTF8len[64]
/* A map from the most-significant 6 bits of the first byte
to the total number of bytes in a UTF-8 character.
 */
= {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* erroneous */
   2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 6};

char* utf8toLatin ( char* cadena) {

int indice = 0;
int tamanyo = strlen(cadena);
int indice_salida = 0;
char* resultado;
/* en el peor de los casos*/
resultado = (char*)malloc (tamanyo * 6 + 1);

while (indice<=tamanyo) 
{
  register int c;
  register unsigned long u;
  auto int len;

  c = (int) cadena[indice];

  len = UTF8len [(c >> 2) & 0x3F];
  indice++;

  switch (len) 
  {
    case 6: u = c & 0x01; break;
    case 5: u = c & 0x03; break;
    case 4: u = c & 0x07; break;
    case 3: u = c & 0x0F; break;
    case 2: u = c & 0x1F; break;
    case 1: u = c & 0x7F; break;
    case 0: /* erroneous: c is the middle of a character. */
      u = c & 0x3F; len = 5; break;
  }

  while (--len && (indice<=tamanyo)) 
  {
     c= (int) cadena[indice];
     indice++;
     if ((c & 0xC0) == 0x80) 
     {
       u = (u << 6) | (c & 0x3F);
     } 
     else { /* unexpected start of a new character */
       indice--;
       break;
     }
  }

  if (u <= 0xFF) 
  {
    //Anyado el caracter a la salida
    resultado[indice_salida++] = (char) u;
    resultado[indice_salida] = '\0';
  }
  else { /* this character can't be represented in Latin-1 */
    resultado[indice_salida++] = '?';
    resultado[indice_salida] = '\0';
  }
  if (indice == tamanyo) break;
	
}
return resultado;
}



// Latin-1 code page
static int s_anLatin1_128to159[] =
{
	8364, 129, 8218, 402, 8222, 8230, 8224, 8225,
	710, 8240, 352, 8249, 338, 141, 381, 143, 144,
	8216, 8217, 8220, 8221, 8226, 8211, 8212, 732,
	8482, 353, 8250, 339, 157, 382, 376
};

string Latin1toUTF8( const char* szStr )
{
	const unsigned char* pSource = (const unsigned char*)szStr;
	string strResult;
	int nLen = strlen( szStr );
	strResult.reserve( nLen + nLen/10 );
	int cSource = *(pSource);
	while ( cSource ){
		if ( cSource >= 128 ){
			if ( cSource <= 159 )	cSource = s_anLatin1_128to159[ cSource - 128 ];
			if ( cSource < 0x800 ){
				strResult += (char)(((cSource&0x7c0)>>6) | 0xc0);
				strResult += (char)((cSource&0x3f) | 0x80);
				}
			else{
				strResult += (char)(((cSource&0xf000)>>12) | 0xe0);
				strResult += (char)(((cSource&0xfc0)>>6) | 0x80);
				strResult += (char)((cSource&0x3f) | 0x80);
			}
		}
		else
			strResult += cSource;
		cSource = *(++pSource);
	}
	return strResult;
}



/// ----------------------------------------------
/// read lines from cin and send them to server

void * send_requests(void *fname) {

  // open server input pipe to write requests
  ofstream fin(((string *)fname)->c_str()); 

  // read all input from cin and send it to the server
  string line;
  while (getline(cin,line)){
	  //unsigned char* utf8toLatin ( char* cadena) {
	  char* aux=utf8toLatin((char*) line.c_str());
	  line=aux;
	  fin << line << endl;  
  }
  fin.close();

  return(NULL); //shut up compiler
}


/// ----------------------------------------------
/// read replies from server and send them to cout

void * get_replies(void *fname) {

 // open server output pipe to read answers
  ifstream fout(((string *)fname)->c_str());

  // get all output from the server and write it to cout
  string line;
  while (getline(fout,line)) {
	  //string Latin1toUTF8( const char* szStr )
	  string aux=Latin1toUTF8(line.c_str());
	  cout << aux <<endl;
  }
  fout.close(); 

  return(NULL); //shut up compiler
}


/// ----------------------------------------------
/// main: just start reader and writer threads

int main(int argc, char **argv) {

  // use first parameter as the name for the named pipe
  string FIFO_in = string(argv[1])+".in";
  string FIFO_out = string(argv[1])+".out";

  pthread_t th_reader, th_writer;
  
  /* Create independent threads each of which will execute function */  
  pthread_create( &th_reader, NULL, send_requests, (void *) &FIFO_in);
  pthread_create( &th_writer, NULL, get_replies, (void *) &FIFO_out);
  
  /* Wait till threads are complete before main continues. Unless we  */
  /* wait we run the risk of executing an exit which will terminate   */
  /* the process and all threads before the threads have completed.   */  
  pthread_join(th_reader, NULL);
  pthread_join(th_writer, NULL); 
}


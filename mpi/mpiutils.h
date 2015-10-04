#ifndef __mpiutils_h__
#define __mpiutils_h__ 1

#include "mpifftw++.h"

namespace fftwpp {

template<class ftype>
void show(ftype *f, unsigned int, unsigned int ny,
          unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
  for(unsigned int i=x0; i < x1; ++i) {
    for(unsigned int j=y0; j < y1; ++j) {
      std::cout << f[ny*i+j]  << "\t";
    }
    std::cout << std::endl;
  }
}

// Gather an MPI-distributed array onto the rank 0 process.  The
// distributed array has dimensions x * Y * Z, the gathered array has
// dimensions X * Y * Z.
template<class ftype>
void gatherx(const ftype *part, ftype *whole,
	     const unsigned int X, const unsigned int Y,
	     const unsigned int x0, const unsigned int y0,
	     const unsigned int x, const unsigned int y,
	     const unsigned int Z,
	     const MPI_Comm& communicator)
{
  MPI_Status stat;
  int size, rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);

  if(rank == 0) {
    // First copy rank 0's part into the whole
    int offset=x0*Y*Z;
    int count=x;
    int length=Y*Z;
    int stride=Y*Z;
    copyfromblock(part,whole+offset,count,length,stride);
      
    for(int p=1; p < size; ++p) {
      unsigned int dims[6];
      MPI_Recv(&dims,6,MPI_UNSIGNED,p,0,communicator,&stat);

      //unsigned int X=dims[0];
      unsigned int Y=dims[1];
      unsigned int x0=dims[2];
      //unsigned int y0=dims[3];
      unsigned int x=dims[4];
      //unsigned int y=dims[5];
      unsigned int n=Z*x*Y;
      if(n > 0) {
        ftype *C=new ftype[n];
        MPI_Recv(C,sizeof(ftype)*n,MPI_BYTE,p,0,communicator,&stat);
	int offset=x0*Y*Z;
	int count=x;
	int length=Y*Z;
	int stride=Y*Z;
	copyfromblock(C,whole+offset,count,length,stride);
	delete [] C;
      }
    }
  } else {
    unsigned int dims[]={X,Y,x0,y0,x,y};
    MPI_Send(&dims,6,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=Z*x*Y;
    if(n > 0)
      MPI_Send((ftype *) part,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}

template<class ftype>
void gatherx(const ftype *part,
	     ftype *whole,
	     const split d,
	     const unsigned int Z,
	     const MPI_Comm& communicator)
{
  gatherx(part,whole,d.X,d.Y,d.x0,d.y0,d.x,d.y,Z,communicator);
}

// Gather an MPI-distributed array onto the rank 0 process.  The
// distributed array has dimensions X * y * Z, the gathered array has
// dimensions X * Y * Z.
template<class ftype>
void gathery(const ftype *part, ftype *whole,
	     const unsigned int X, const unsigned int Y,
	     const unsigned int x0, const unsigned int y0,
	     const unsigned int x, const unsigned int y,
	     const unsigned int Z,
	     const MPI_Comm& communicator)
{
  MPI_Status stat;
  int size, rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);

  if(rank == 0) {
    // First copy rank 0's part into the whole
    int offset=y0*Z;
    int count=X;
    int length=y*Z;
    int stride=Y*Z;
    copyfromblock(part,whole+offset,count,length,stride);
      
    for(int p=1; p < size; ++p) {
      unsigned int dims[6];
      MPI_Recv(&dims,6,MPI_UNSIGNED,p,0,communicator,&stat);

      unsigned int X=dims[0];
      unsigned int Y=dims[1];
      //unsigned int x0=dims[2];
      unsigned int y0=dims[3];
      //unsigned int x=dims[4];
      unsigned int y=dims[5];
      unsigned int n=Z*X*y;
      if(n > 0) {
        ftype *C=new ftype[n];
        MPI_Recv(C,sizeof(ftype)*n,MPI_BYTE,p,0,communicator,&stat);
	int offset=y0*Z;
	int count=X;
	int length=y*Z;
	int stride=Y*Z;
	copyfromblock(C,whole+offset,count,length,stride);
	delete [] C;
      }
    }
  } else {
    unsigned int dims[]={X,Y,x0,y0,x,y};
    MPI_Send(&dims,6,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=Z*X*y;
    if(n > 0)
      MPI_Send((ftype *) part,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}

template<class ftype>
void gathery(const ftype *part,
	     ftype *whole,
	     const split d,
	     const unsigned int Z,
	     const MPI_Comm& communicator)
{
  gathery(part,whole,d.X,d.Y,d.x0,d.y0,d.x,d.y,Z,communicator);
}

// Gather an MPI-distributed array onto the rank 0 process.  The
// distributed array has dimensions X * y * z, the gathered array has
// dimensions X * Y * Z.
template<class ftype>
void gatheryz(const ftype *part,
	      ftype *whole,
	      const unsigned int X,
	      const unsigned int Y,
	      const unsigned int Z,
	      const unsigned int x0,
	      const unsigned int y0,
	      const unsigned int z0,
	      const unsigned int x,
	      const unsigned int y,
	      const unsigned int z,
	      const MPI_Comm& communicator)
{
  MPI_Status stat;
  int size, rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);

  //  X * y * z
  if(rank == 0) {
    // First copy rank 0's part into the whole
    const int count=y;
    const int stride=Z;
    const int length=z;
    for(unsigned int i=0; i < X; ++i) {
      const int outoffset=i*Y*Z+y0*Z+z0;
      const int inoffset=i*y*z;
      copyfromblock(part+inoffset,whole+outoffset,
		    count,length,stride);
    }
    for(int p=1; p < size; ++p) {
      unsigned int dims[9];
      MPI_Recv(&dims,9,MPI_UNSIGNED,p,0,communicator,&stat);
      unsigned int X=dims[0];
      unsigned int Y=dims[1];
      unsigned int Z=dims[2];
      //unsigned int x0=dims[3];
      unsigned int y0=dims[4];
      unsigned int z0=dims[5];
      //unsigned int x=dims[6];
      unsigned int y=dims[7];
      unsigned int z=dims[8];

      unsigned int n=X*y*z;
      if(n > 0) {
	ftype *C=new ftype[n];
	MPI_Recv(C,sizeof(ftype)*n,MPI_BYTE,p,0,communicator,&stat);
	const int count=y;
	const int stride=Z;
	const int length=z;
	for(unsigned int i=0; i < X; ++i) {
	  const int outoffset=i*Y*Z+y0*Z+z0;
	  const int inoffset=i*y*z;
	  copyfromblock(C+inoffset,whole+outoffset,count,length,stride);
	}
	delete [] C;
      }
    }
  } else {
    unsigned int dims[9] = {X,Y,Z,x0,y0,z0,x,y,z};
    MPI_Send(&dims,9,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=X*y*z;
    if(n > 0)
      MPI_Send((ftype *) part,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}
 
template<class ftype>
void gatheryz(const ftype *part,
	      ftype *whole,
	      const split3 d,
	      const MPI_Comm& communicator)
{
  gatheryz(part, whole,
	   d.X,d.Y,d.Z,
	   d.x0,d.xy.y0,d.z0,
	   d.x,d.xy.y,d.z,
	   communicator);
}

// Gather an MPI-distributed array onto the rank 0 process.  The
// distributed array has dimensions x * y * Z, the gathered array has
// dimensions X * Y * Z.
template<class ftype>
void gatherxy(const ftype *part,
	      ftype *whole,
	      const split3 d,
	      const MPI_Comm& communicator)
{
  MPI_Status stat;
  int size, rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);

  const unsigned int X=d.x;
  const unsigned int Y=d.Y;
  const unsigned int Z=d.Z;
  const unsigned int x0=d.x0;
  const unsigned int y0=d.y0;
  const unsigned int z0=d.z0;
  const unsigned int x=d.x;
  const unsigned int y=d.y;
  const unsigned int z=d.z;
                
  // x * y * Z
  if(rank == 0) {
    // First copy rank 0's part into the whole
    const int count=y;
    const int stride=Z;
    const int length=Z;
    for(unsigned int i=0; i < x; ++i) {
      const int poffset=i*y*Z;
      const int woffset=(x0+i)*Y*Z+y0*Z;
      copyfromblock(part+poffset,whole+woffset,
		    count,length,stride);
    }
    for(int p=1; p < size; ++p) {
      unsigned int dims[9];
      MPI_Recv(&dims,9,MPI_UNSIGNED,p,0,communicator,&stat);
      //unsigned int X=dims[0];
      unsigned int Y=dims[1];
      unsigned int Z=dims[2];
      unsigned int x0=dims[3];
      unsigned int y0=dims[4];
      //unsigned int z0=dims[5];
      unsigned int x=dims[6];
      unsigned int y=dims[7];
      //unsigned int z=dims[8];

      unsigned int n=x*y*Z;
      if(n > 0) {
	ftype *C=new ftype[n];
	MPI_Recv(C,sizeof(ftype)*n,MPI_BYTE,p,0,communicator,&stat);
	const int count=y;
	const int stride=Z;
	const int length=Z;
	for(unsigned int i=0; i < x; ++i) {
	  const int poffset=i*y*Z;
	  const int woffset=(x0+i)*Y*Z+y0*Z;
	  copyfromblock(C+poffset,whole+woffset,
			count,length,stride);

	}
	delete [] C;
      }
    }
  } else {
    unsigned int dims[9] = {X,Y,Z,x0,y0,z0,x,y,z};
    MPI_Send(&dims,9,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=x*y*Z;
    if(n > 0)
      MPI_Send((ftype *) part,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}

template<class T>
int checkerror(const T *f, const T *control, unsigned int stop)
{
  double maxerr=0.0, norm=0.0;
  for(unsigned int i=0; i < stop; i++) {
    maxerr=std::max(maxerr,abs(f[i]-control[i]));
    norm=std::max(norm,abs(control[i]));
  }
  std::cout << "Maximum error: " << maxerr << std::endl;
  if(maxerr <= 1e-12*norm) {
    std::cout << "Error ok." << std::endl;
    return 0;
  }
  std::cout << "CAUTION! Large error!" << std::endl;
    return 1;
}

// output the contents of a 2D array
template<class ftype>
void show(ftype *f, unsigned int nx, unsigned int ny,
          unsigned int x0, unsigned int y0,
          unsigned int x1, unsigned int y1, const MPI_Comm& communicator)
          
{  
  MPI_Status stat;
  int size,rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);
  
  if(rank == 0) {
    std::cout << "process " << 0 << ":" <<  std::endl;
    show(f,nx,ny,x0,y0,x1,y1);
    
    for(int p=1; p < size; p++) {
      unsigned int dims[6];
      MPI_Recv(&dims,6,MPI_UNSIGNED,p,0,communicator,&stat);

      unsigned int nx=dims[0], ny=dims[1];
      unsigned int x0=dims[2], y0=dims[3];
      unsigned int x1=dims[4], y1=dims[5];
      unsigned int n=nx*ny;
      std::cout << "process " << p << ":" <<  std::endl;
      if(n > 0) {
        ftype *C=new ftype[n];
        MPI_Recv(C,sizeof(ftype)*n,MPI_BYTE,p,0,communicator,&stat);
      
        show(C,nx,ny,x0,y0,x1,y1);
        delete [] C;
      }
    }
  } else {
    unsigned int dims[]={nx,ny,x0,y0,x1,y1};
    MPI_Send(&dims,6,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=nx*ny;
    if(n > 0)
      MPI_Send(f,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}
  
template<class ftype>
void show(ftype *f, unsigned int nx, unsigned int ny,
          const MPI_Comm& communicator)
{ 
  show(f,nx,ny,0,0,nx,ny,communicator);
}

template<class ftype>
void show(ftype *f, unsigned int nx, unsigned int ny, unsigned int nz,
          unsigned int x0, unsigned int y0, unsigned int z0,
          unsigned int x1, unsigned int y1, unsigned int z1)
{
  for(unsigned int i=x0; i < x1; ++i) {
    for(unsigned int j=y0; j < y1; ++j) {
      for(unsigned int k=z0; k < z1; ++k) {
        std::cout << f[nz*(ny*i+j)+k]  << "\t";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

// output the contents of a 3D array
template<class ftype>
void show(ftype *f, unsigned int nx, unsigned int ny, unsigned int nz,
          unsigned int x0, unsigned int y0, unsigned int z0,
          unsigned int x1, unsigned int y1, unsigned int z1,
          const MPI_Comm& communicator)
{
  MPI_Status stat;
  int size,rank;
  MPI_Comm_size(communicator,&size);
  MPI_Comm_rank(communicator,&rank);
  
  if(rank == 0) {
    std::cout << "process " << 0 << ":" <<  std::endl;
    show(f,nx,ny,nz,x0,y0,z0,x1,y1,z1);
    
    for(int p=1; p < size; p++) {
      unsigned int dims[9];
      MPI_Recv(&dims,9,MPI_UNSIGNED,p,0,communicator,&stat);

      unsigned int nx=dims[0], ny=dims[1], nz=dims[2];
      unsigned int x0=dims[3], y0=dims[4], z0=dims[5];
      unsigned int x1=dims[6], y1=dims[7], z1=dims[8];
      unsigned int n=nx*ny*nz;
      if(n > 0) {
        ftype *C=new ftype[n];
        MPI_Recv(C,n*sizeof(ftype),MPI_BYTE,p,0,communicator,&stat);
      
        std::cout << "process " << p << ":" <<  std::endl;
        show(C,nx,ny,nz,x0,y0,z0,x1,y1,z1);
        delete [] C;
      }
    }
  } else {
    unsigned int dims[]={nx,ny,nz,x0,y0,z0,x1,y1,z1};
    MPI_Send(&dims,9,MPI_UNSIGNED,0,0,communicator);
    unsigned int n=nx*ny*nz;
    if(n > 0)
      MPI_Send(f,n*sizeof(ftype),MPI_BYTE,0,0,communicator);
  }
}

template<class ftype>
void show(ftype *f, unsigned int nx, unsigned int ny, unsigned int nz,
          const MPI_Comm& communicator)
{ 
  show(f,nx,ny,nz,0,0,0,nx,ny,nz,communicator);
}

// hash-check for 2D arrays
int hash(Complex *f, unsigned int nx, unsigned int ny,
         const MPI_Comm& communicator);

// return a hash of the contents of a 3D complex array
int hash(Complex *f, unsigned int nx, unsigned int ny, unsigned int nz,
         MPI_Comm communicator);

} // namespace fftwpp

#endif
  

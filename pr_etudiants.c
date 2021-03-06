/* PageRank 
   The genetic.dat dataset comes from:
   http://www.cs.toronto.edu/~tsap/experiments/datasets/
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* allocate one object of given type */
#define NEW(type) ((type*)calloc((size_t)1,(size_t)sizeof(type)))

/* allocate num objects of given type */
#define NEW_A(num,type) ((type*)calloc((size_t)(num),(size_t)sizeof(type)))

typedef unsigned int u_int;

/* vector */
typedef struct
{
  u_int dim;
  double *e;
} VEC;

/* matrix */
typedef struct
{
  u_int m, n;
  double **e;
} MAT;

/* v_get -- gets a VEC of dimension 'dim'
   Precondition: size >= 0
   Postcondition: initialized to zero */
VEC *v_get( u_int size )
{
  VEC *v;
  
  if( (v = NEW(VEC)) == (VEC *)NULL )
  {
    fprintf( stderr, "v_get memory error" );
    exit( -1 );
  }
  
  v->dim = size;
  if( (v->e = NEW_A(size,double)) == (double *)NULL )
  {
    free( v );
    fprintf( stderr, "v_get memory error" );
    exit( -1 );
  }
  
  return v;
}

/* v_free -- returns VEC & associated memory back to memory heap */
int v_free( VEC *v )
{
  if( v == (VEC *)NULL )
    return -1;
  
  if( v->e == (double *)NULL ) 
  {
    free( v );
  }
  else
  {
    free( v->e );
    free( v );
  }
  
  return 0;
}

/* m_get -- gets an mxn matrix by dynamic memory allocation 
   Precondition: m>=0 && n>=0
   Postcondition: initialized to zero */
MAT *m_get( u_int m, u_int n )
{
  u_int i;
  MAT *M;
  
  if( (M = NEW(MAT)) == (MAT *)NULL )
  {
    fprintf( stderr, "m_get memory error" );
    exit( -1 );
  }
  
  M->m = m ; M->n = n;

  if( (M->e = NEW_A(m,double*)) == (double **)NULL )
  {
    free( M );
    fprintf( stderr, "m_get memory error" );
    exit( -1 );
  }
  
  for( i = 0 ; i < m ; i++ )
  {
    if( (M->e[i] = NEW_A(n,double)) == (double *)NULL )
    {
      fprintf( stderr, "m_get memory error" );
      exit( -1 );
    }
  }
  
  return M;
}

/* m_free -- returns MAT & associated memory back to memory heap */
int m_free( MAT *M )
{
  u_int i;

  if( M == (MAT *)NULL ) return -1;
  
  for( i = 0 ; i < M->m ; i++ )
  {
    if( M->e[i] != (double *)NULL ) free( M->e[i] );
  }

  if( M->e != (double **)NULL ) free( M->e );
  
  free( M );
  
  return 0;
}

/* m_input -- file input of matrix */
MAT *m_input( FILE *fp )
{
  MAT *M;
  u_int i,j,m,n,val;
  
  /* get dimension */
  if( fscanf( fp, " Matrix: %u by %u", &m, &n ) < 2 )
  {
    fprintf( stderr, "m_input error reading dimensions" );
    exit( -1 );
  }
  
  /* allocate memory */
  M = m_get( m, n );
  
  /* get entries */
  for( i = 0 ; i < m ; i++ )
  {
    if( fscanf( fp, " row %u:", &val ) < 1 )
    {
      fprintf( stderr, "m_input error reading line %u", i );
      exit( -1 );
    }
    for( j=0 ; j<n ; j++ )
    {
      if( fscanf( fp, "%lf", &M->e[i][j] ) < 1 )
      {
        fprintf( stderr, "m_input error reading line %u col %u", i, j );
        exit( -1 );
      }
    }
  }
  
  return M;
}

/* m_output -- file output of a matrix 
   Precondition: Memory already allocated for the matrix */
void m_output( FILE *fp, MAT *M )
{
  u_int i,j;

  fprintf( fp, "Matrix: %d by %d\n", M->m, M->n );
  for( i = 0 ; i < M->m ; i++ )
  {
    fprintf( fp, "row %u: ", i);
    for( j = 0 ; j < M->n ; j++ )
    {
      fprintf( fp, "%1.5g ", M->e[i][j] );
    }
    putc( '\n', fp );
  }
}

/* v_output -- file output of vector */
void v_output( FILE *fp, VEC *v )
{
  u_int i;

  fprintf( fp, "Vector: %d\n", v->dim );
  for( i = 0 ; i < v->dim ; i++ ) fprintf( fp, "%1.5g ", v->e[i] );
  putc( '\n', fp );
}

/*******************************************************
  Sparse Matrix section: representation by adjacency lists
  Only useful for the last part of the practical work.
********************************************************/

/* row of a sparse matrix, i.e. adjacency list */
typedef struct
{
  u_int  nnz;  /* # of non-zero (nz) value on this row */
  u_int  *col; /* column identifier for each nz value */
  double *val; /* value for each nz value */
} SROW;

/* sparse matrix */
typedef struct
{
  u_int m, n;
  SROW *row;
} SMAT;

/* sm_get -- gets an mxn sparse matrix by dynamic memory allocation 
   Precondition: m>=0 && n>=0
   Postcondition: each row is empty*/
SMAT *sm_get( u_int m, u_int n )
{
  SMAT *M;
  u_int i;
  
  if( (M = NEW(SMAT)) == (SMAT *)NULL )
  {
    fprintf( stderr, "sm_get memory error" );
    exit( -1 );
  }
  
  M->m = m ; M->n = n;

  if( (M->row = NEW_A(m,SROW)) == (SROW *)NULL )
  {
    free( M );
    fprintf( stderr, "sm_get memory error" );
    exit( -1 );
  }
  
  for( i = 0 ; i < m ; i++ )
  {
    (M->row[i]).nnz = 0;
    (M->row[i]).col = (u_int *) NULL;
    (M->row[i]).val = (double *) NULL;
  }

  return M;
}

/* sm_free -- returns SMAT & associated memory back to memory heap */
int sm_free( SMAT *M )
{
  u_int i;
  SROW *ri;

  if( M == (SMAT *)NULL ) return -1;
  
  if( M->row == (SROW *)NULL ) 
  {
    free( M );
  }
  else
  {
    for( i = 0 ; i < M->m ; i++ )
    {
      ri = &(M->row[i]);
      if( ri->nnz > 0 )
      {
        free( ri->col );
        free( ri->val );
      }
    }
    free( M->row );
    free( M );
  }
  
  return 0;
}

/* sm_input -- file input of sparse matrix 
   Precondition: will only work with a binary matrix. */
SMAT *sm_input( FILE *fp )
{
  SMAT *M;
  u_int *col; /* temp array to store the nz col of the current row */
  u_int r;    /* index of current row */
  int c;      /* index of current column */
  SROW *ri;   /* pointer to the current row in M */
  u_int m,n,i,j,k;
  
  /* get dimension */
  if( fscanf( fp, " SparseMatrix: %u by %u", &m, &n ) < 2 )
  {
    fprintf( stderr, "sm_input error reading dimensions" );
    exit( -1 );
  }
  
  if( (col = NEW_A(n,u_int)) == (u_int *)NULL )
  {
    fprintf( stderr, "sm_input memory error" );
    exit( -1 );
  }

  M = sm_get( m, n );
  
  /* get entries */
  for( i=0 ; i<m ; i++ )
  {
    if( fscanf( fp, " row %u:", &r ) < 1 )
    {
      fprintf( stderr, "sm_input error reading line %u", i );
      exit( -1 );
    }
    ri = &(M->row[i]);
    j = 0;
    for( ; ; )
    {
      if( fscanf( fp, "%d", &c ) < 1 )
      {
        fprintf( stderr, "sm_input error reading line %u col x", i );
        exit( -1 );
      }
      if( c < 0 ) break;
      col[j] = c;
      j++;
    } /* j is the number of nz value in row i */

    if( ( (ri->col = NEW_A(j,u_int)) == (u_int *)NULL ) && ( j!=0 ) )
    {
      fprintf( stderr, "sm_input memory error" );
      exit( -1 );
    }
    if( ( (ri->val = NEW_A(j,double)) == (double *)NULL ) && ( j!=0 ) )
    {
      fprintf( stderr, "sm_input memory error" );
      exit( -1 );
    }

    ri->nnz = j;

    for( k = 0 ; k < j ; k++ )
    {
      ri->col[k] = col[k]; 
      ri->val[k] = 1.0;
    }
  }
  
  free( col );

  return M;
}

/* sm_output -- file output of sparse matrix 
   Postcondition: the result is not a valid entry for sm_input,
     since it also works for a non-binary matrix. */
void sm_output( FILE *fp, SMAT *M )
{
  u_int i,j;
  SROW *ri;

  fprintf( fp, "SparseMatrix: %d by %d\n", M->m, M->n );

  for( i = 0 ; i < M->m ; i++ )
  {
    fprintf( fp, "row %u: ", i ); 
    ri = &( M->row[i] );
    for( j = 0 ; j < ri->nnz ; j++ )
    {
      fprintf( fp, "%u:%1.5g ", ri->col[j], ri->val[j] );
    }
    fprintf( fp, "-1\n" );
  }
}

/* End of Sparse Matrix section. */
/*********************************/
void initH(MAT* M){

  u_int i;
  u_int j;
  u_int sum;

  for(i=0;i<M->m;i++){
    sum=0;
    for(j=0;j<M->n;j++){
      sum=sum+M->e[i][j];
    }
    if (sum==0){
      for(j=0;j<M->n;j++){
        M->e[i][j]=(double) 1/M->n;
      }
    }else{
      for(j=0;j<M->n;j++){
        M->e[i][j]=M->e[i][j]/sum;
      }
    }
  }

}

void iteratematmat( MAT * R, MAT * H)
{
	int somme;

	int i;
	int j;
	int k;
	for (i = 0; i < R->m; ++i)
	{
		for (j = 0; j < R->n; ++j)
		{
			somme = 0;
			for (k = 0; k < R->n; ++k)
			{
				somme += (R->e[i][k]) * (H->e[k][j]);
			}
			R->e[i][j] = somme;
		}
	}
}


  
void iteratevectmat( VEC * R, MAT * H)
{
  double somme;
  VEC * Rcopy = v_get(R->dim);

  int i;
  int j;
  for (i = 0; i < R->dim; ++i)
  {
    somme = 0;
    for (j = 0; j < R->dim; ++j)
    {
      somme += R->e[j] * H->e[j][i];
    }
    Rcopy->e[i] = somme;
  }

  //v_output(stdout,Rcopy);

  for (i = 0; i < R->dim; ++i)
  {
     R->e[i] = Rcopy->e[i];
  }

  v_free(Rcopy);
}

void ergodique(MAT * M, double alpha)
{
  int i;
  int j;
  for (j = 0; j < M->m; ++j)
  {
    for (i = 0; i < M->n; ++i)
    {
      M->e[i][j] = alpha * M->e[i][j] + (1 - alpha) / (M->m);
    }
  }
}

int main()
{
  FILE *fp;
  MAT *M;
  VEC *R;

  fp = fopen( "g.dat", "r" );
  M = m_input( fp );
  fclose( fp );

  m_output( stdout, M );
  initH(M);
  m_output( stdout, M);

  //initializing R to R0

  R=v_get(M->n);
  int i;
  for(i=0;i<R->dim;i++){
    R->e[i]=(double) 1/(R->dim);
  }

  v_output(stdout, R);

  ergodique(M, 0.5);

  m_output(stdout, M);

  for (i = 0; i < 10; ++i)
  {
    iteratevectmat(R, M);
    v_output(stdout, R);
  }



  /*for (i = 0; i < 10; ++i)
  {
    iteratevectmat(R, M);
  }

  v_output(stdout, R);
  */

  // free everything
  m_free( M );
  v_free(R);


  /* Working with sparse matrix */
  /*
  SMAT *SM;
  fp = fopen( "genetic.dat", "r" );
  SM = sm_input( fp );
  fclose( fp );
  fp = fopen( "test.dat", "w" );
  sm_output( fp, SM );
  sm_free( SM );
*/

  return 0;
}
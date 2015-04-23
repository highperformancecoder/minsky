/*
  Compare two files, ignoring white space, returning 0 if they match
and 1 if they differ (like diff). But in contrast to diff, any numbers
in the files are converted to floating point numbers and compared to
be within relative error of 1e-3.
*/
#include <stdio.h>
#include <ctype.h>
#include <math.h>

int nextNonWS(FILE* f)
{
  int c;
  for (c=fgetc(f); c!=EOF && isspace(c); c=fgetc(f));
  return c;
}

int main(int argc, const char** argv)
{
  if (argc<3) return 1;

  FILE* f1=fopen(argv[1],"r");
  FILE* f2=fopen(argv[2],"r");
  int c1, c2;
  
  while ((c1=nextNonWS(f1))!=EOF && (c2=nextNonWS(f2))!=EOF)
    {
      if (isdigit(c1) && isdigit(c2))
        {
          ungetc(c1,f1);
          ungetc(c2,f2);
          float d1, d2;
          fscanf(f1,"%g",&d1);
          fscanf(f2,"%g",&d2);
          if (d1*d2==0)
            {
              if (fabs(d1-d2)>1e-30)
                return 1;
            }
          else if (fabs(d1-d2)>1e-3*(fabs(d1)+fabs(d2)))
            return 1;
        }
      else
        if (c1!=c2)
          return 1;
    }

  return (nextNonWS(f2)==EOF)? 0: 1;
}

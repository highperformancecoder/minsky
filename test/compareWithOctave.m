example=argv(){1};
source ([example ".m"]);
source ([example ".dat"]);

function h=heaviside(x)
  if (x<0)
    h = 0;
  else 
    h = 1;
  endif
endfunction

# split input data into timestep vector and data to compare
t=d(:,1);
xc=d(:,2:size(d,2));
x=lsode(@f, x0, t);

if (any(any(abs(x)>1e-30 & abs(x-xc) > 0.1*(abs(x)+abs(xc)) | abs(x)<=1e-30 & abs(xc)>1e-30)))
  [i,j]=find(abs(x-xc) > 0.1*(abs(x)+abs(xc)));
  simulation_differs_at_t=t(unique(i))
  exit(1)
endif

require,"yoga.i";
require,"util_fr.i";

/*
  ____ _   _ _____ ____ _  ______  
 / ___| | | | ____/ ___| |/ / ___| 
| |   | |_| |  _|| |   | ' /\___ \ 
| |___|  _  | |__| |___| . \ ___) |
 \____|_| |_|_____\____|_|\_\____/ 
                                   
 */

func check_yoga_sparse_obj(size)
{
  if (is_void(size)) size= 512;

  matA=random(size, size);

  mask=int(random(size,size)+0.2); nz=numberof(where(mask==0));
  write, format="putting %d zeros %0.2f %% of the matrix\n", nz, float(nz)/(size*size)*100;
  matA*=mask;
  
  matA_gpu = yoga_sparse_obj(matA);
  matA_gpu;

  write, "max(abs(matA_gpu()-matA))";
  max(abs(matA_gpu()-matA));

  checkall_yoga_cusparse;
  
}


func checkall_yoga_cusparse(size)
{
  write,"checkMV_sparse";
  "";
  checkMV_sparse,size;
  "";
  write,"checkMM_sparse";
  "";
  checkMM_sparse,size;
}

func checkMV_sparse(sizem, sizen,ctype=)
/* DOCUMENT checkMV(sizem, sizen,ctype=)
     this templates does the profiling of matrix-vector multiply
   SEE ALSO:
 */
{
  if (is_void(ctype)) ctype = "all";
  if(is_void(sizem)) sizem=512;
  if(is_void(sizen)) sizen=sizem;
  
  matA=random(sizem, sizen);  vectx=random(sizen);
  vecty=array(0., sizem);

  mask=int(random(sizem,sizen)+0.2); nz=numberof(where(mask==0));
  write, format="putting %d zeros %0.2f %% of the matrix\n", nz, float(nz)/(sizem*sizen)*100;
  matA*=mask;
  
  if ((ctype == "all") || (ctype == "gpu")) {
    timeTic = tic();
    timeProfile=[];
    
    matA_gpu = yoga_sparse_obj(matA);
    matA_gpu;
    grow, timeProfile, tac(timeTic);
    vectx_gpu = yoga_obj(vectx);
    grow, timeProfile, tac(timeTic);
    vecty_gpu = yoga_obj(vecty);
    grow, timeProfile, tac(timeTic);
    
    yoga_mv_sparse,vecty_gpu,matA_gpu,vectx_gpu;
    grow, timeProfile, tac(timeTic);
  
    vectyb=vecty_gpu();
    grow, timeProfile, tac(timeTic);

    matA_gpu = vectx_gpu = vecty_gpu = [];
    grow, timeProfile, tac(timeTic);
    "";
    "MV mult gpu time profile: ";
    "h2dm     h2dv1    h2dv2    comp    d2hv    free"  
    timeProfile; 
    "";
    "MV mult gpu time individual: ";
    "h2dv1       h2dv2       comp        d2hv       free"  
    timeProfile(dif); 
    "";
  }
  
  if ((ctype == "all") || (ctype == "cpu")) {
    timeTic=tic();
    vecty=matA(,+)*vectx(+);
    write,format="MV mult cpu time : %f\n",tac(timeTic);
    if (ctype != "all") timeProfile = tac(timeTic);
    else {
      timecpu=tac(timeTic);
      write,format="error for mv : %f\n",max(abs(vecty-vectyb));
      tmp = timeProfile;
      "";
      write,format="Acceleration factor w memcopy : %.1f\n",timecpu/tmp(1:4)(sum);
      write,format="Acceleration factor w/ memcopy : %.1f\n",timecpu/tmp(dif)(3);
    }
  }
  return timeProfile;
}

func checkMM_sparse(sizem, sizen, sizek,ctype=)
/* DOCUMENT checkMM(sizem, sizen, sizek,ctype=)
     this templates does the profiling of matrix-matrix multiply
   SEE ALSO:
 */
{
  if (is_void(ctype)) ctype = "all";
  if(is_void(sizem)) sizem=512;
  if(is_void(sizen)) sizen=sizem;
  if(is_void(sizek)) sizek=sizem;
  
  matA=float(random(sizem, sizek)); matB=float(random(sizek, sizen));
  matC=matCb=array(float, dimsof(matA)(2), dimsof(matB)(3));

  mask=int(random(sizem,sizen)+0.2); nz=numberof(where(mask==0));
  write, format="putting %d zeros %0.2f %% of the matrix\n", nz, float(nz)/(sizem*sizen)*100;
  matA*=mask;

  if ((ctype == "all") || (ctype == "gpu")) {
    timeTic = tic();
    timeProfile=[];
    
    matA_gpu = yoga_sparse_obj(matA);
    matA_gpu;
    grow, timeProfile, tac(timeTic);
    matB_gpu = yoga_obj(matB);
    grow, timeProfile, tac(timeTic);
    matC_gpu = yoga_obj(matC);
    grow, timeProfile, tac(timeTic);
  
    yoga_mm_sparse,matC_gpu,matA_gpu,matB_gpu;
    grow, timeProfile, tac(timeTic);
  
    matCb=matC_gpu();
    grow, timeProfile, tac(timeTic);
    
    matA_gpu = matB_gpu = matC_gpu;
    grow, timeProfile, tac(timeTic);
    
    "";
    "MM mult gpu time profile: ";
    "h2dm     h2dv1    h2dv2    comp    d2hv    free"  
    timeProfile; 
    "";
    "MM mult gpu time individual: ";
    "h2dm2       h2dm3       comp        d2hm       free"  
    timeProfile(dif); 
    "";
  }
  
  if ((ctype == "all") || (ctype == "cpu")) {
    "matD=matA(,+)*matB(+,)";
    timeTic=tic();
    matD=matA(,+)*matB(+,); 
    write,format="MM cpu time : %f\n",tac(timeTic);

    if (ctype != "all") timeProfile = tac(timeTic);
    else {
      write,format="error for mm : %f\n",max(abs(matCb-matD));
      timecpu=tac(timeTic);
      tmp = timeProfile;
      "";
      write,format="Acceleration factor w memcopy : %.1f\n",timecpu/tmp(1:4)(sum);
      write,format="Acceleration factor w/ memcopy : %.1f\n",timecpu/tmp(dif)(3);
    }
  }
  return timeProfile;
}

#include <sutra_controller.h>
#include <string>

sutra_controller::sutra_controller(carma_context* context, int nslope,
    int nactu) {
  this->current_context = context;
  this->device = context->get_activeDevice();
  //current_context->set_activeDevice(device,1);

  int nstreams = 1; //nvalid/10;
  while (nactu % nstreams != 0)
    nstreams--;

  cerr << "controller uses " << nstreams << " streams" << endl;
  streams = new carma_streams(nstreams);

  long dims_data1[2] = { 1, 0 };

  dims_data1[1] = nslope;
  this->d_centroids = new carma_obj<float>(context, dims_data1);
  dims_data1[1] = nactu;
  this->d_com = new carma_obj<float>(context, dims_data1);

}

sutra_controller::~sutra_controller() {
  delete this->streams;

  delete this->d_centroids;
  delete this->d_com;
}
int
sutra_controller::syevd_f(char meth, carma_obj<float> *d_U, carma_host_obj<float> *h_eigenvals){
  current_context->set_activeDevice(device,1);
	// Init double arrays
	const long dims_data[3] = {2, d_U->getDims()[1], d_U->getDims()[2]};
	carma_obj<double> *d_Udouble = new carma_obj<double>(current_context,dims_data);
	const long dims_data2[2] = {1, h_eigenvals->getDims()[1]};
	carma_host_obj<double> *h_eigen_double = new carma_host_obj<double>(dims_data2,MA_PAGELOCK);

	// Copy float array in double array
	floattodouble(d_U->getData(),d_Udouble->getData(),d_U->getNbElem(), this->current_context->get_device(device));

	//Doing syevd<double>
	carma_syevd<double,1>(meth,d_Udouble,h_eigen_double);

	// Reverse copy
	doubletofloat(d_Udouble->getData(),d_U->getData(),d_U->getNbElem(), this->current_context->get_device(device));
	for(int cc=0 ; cc<h_eigenvals->getNbElem() ; cc++){
		h_eigenvals->getData()[cc] = (float)h_eigen_double->getData()[cc];
	}

	delete d_Udouble;
	delete h_eigen_double;

	return EXIT_SUCCESS;
}
int
sutra_controller::invgen(carma_obj<float> *d_mat, float cond, int job){

  current_context->set_activeDevice(device,1);
	const long dims_data[3] = {2, d_mat->getDims()[1], d_mat->getDims()[2]};
	carma_obj<float> *d_U = new carma_obj<float>(current_context,dims_data);
	carma_obj<float> *d_tmp = new carma_obj<float>(current_context,dims_data);
	int i;

	const long dims_data2[2]={1,d_mat->getDims()[1]};
	carma_obj<float> *d_eigenvals_inv = new carma_obj<float>(current_context,dims_data2);
	carma_host_obj<float> *h_eigenvals = new carma_host_obj<float>(dims_data2, MA_PAGELOCK);
	carma_host_obj<float> *h_eigenvals_inv = new carma_host_obj<float>(dims_data2, MA_PAGELOCK);

	d_U->copy(d_mat,1,1);
	carma_syevd<float,1>('V',d_U,h_eigenvals);
	//syevd_f('V',d_U,h_eigenvals);
	if (job == 1){ // Conditionnement
		float maxe = h_eigenvals->getData()[d_mat->getDims()[1] - 1];

		for (i=0 ; i<d_mat->getDims()[1] ; i++){
			if(h_eigenvals->getData()[i] < maxe/cond)
				h_eigenvals_inv->getData()[i] = 0.;
			else
				h_eigenvals_inv->getData()[i] = 1./h_eigenvals->getData()[i];
		}
	}
	if (job == 0){ // Filtre #cond modes
		for (i=0 ; i<d_mat->getDims()[1] ; i++){
					if(i < cond)
						h_eigenvals_inv->getData()[i] = 0.;
					else
						h_eigenvals_inv->getData()[i] = 1./h_eigenvals->getData()[i];
				}
	}

	d_eigenvals_inv->host2device(*h_eigenvals_inv);

	carma_dgmm(cublas_handle(),CUBLAS_SIDE_RIGHT,d_mat->getDims()[1],d_mat->getDims()[2],
			d_U->getData(),d_mat->getDims()[1],d_eigenvals_inv->getData(),1,
			d_tmp->getData(),d_mat->getDims()[1]);
	carma_gemm<float>(cublas_handle(), 'n', 't', d_mat->getDims()[1], d_mat->getDims()[1], d_mat->getDims()[2], 1.0f,
			      d_tmp->getData(), d_mat->getDims()[1], d_U->getData(), d_mat->getDims()[1], 0.0f,
			      d_mat->getData(), d_mat->getDims()[1]);

	delete d_U;
	delete d_tmp;
	delete d_eigenvals_inv;
	delete h_eigenvals;
	delete h_eigenvals_inv;


	return EXIT_SUCCESS;
}

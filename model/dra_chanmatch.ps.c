/* dra_chanmatch.ps.c */                                                       
/* Default channel match model for radio link Transceiver Pipeline */

/****************************************/
/*		  Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

#include "opnet.h"


#if defined (__cplusplus)
extern "C"
#endif
void
dra_chanmatch_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
	{
	double		tx_freq, tx_bw, tx_drate, tx_code;
	double 		tx_x,tx_y,tx_z,rx_x,rx_y,rx_z,dif_x,dif_y,dif_z,dif_mag;
	double		cos_lambda,sin_lambda,cos_phi,sin_phi,cos_theta,sin_theta;
	double		alpha,gamma,sin_alpha,cos_alpha,sin_gamma,cos_gamma,arg_phi;
	double		y,y1,y2,y3,D,h,lambda,phi,beam_width;
	double		rx_freq, rx_bw, rx_drate, rx_code;
	Vartype		tx_mod;
	Vartype		rx_mod;
	Objid obj_rx = 0,obj_tx = 0;
	int rx_id = 0,tx_id = 0;
	double rx_gamma = 0,tx_gamma = 0,rx_alpha = 0,tx_alpha = 0;
	int is_match = 1;
	char tx_pa[256],rx_pa[256];

	/** Determine the compatibility between transmitter and receiver channels. **/
	FIN_MT (dra_chanmatch (pkptr));

	/* Obtain transmitting channel attributes. */
	tx_freq		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_FREQ);
	tx_bw		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_BW);
	tx_drate	= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_DRATE);
	tx_code		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_CODE);
	tx_mod		= op_td_get_ptr (pkptr, OPC_TDA_RA_TX_MOD);	

	/* Obtain receiving channel attributes. */
	rx_freq		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_FREQ);
	rx_bw		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_BW);
	rx_drate	= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_DRATE);
	rx_code		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_CODE);
	rx_mod		= op_td_get_ptr (pkptr, OPC_TDA_RA_RX_MOD);	

	obj_rx = op_td_get_int(pkptr,OPC_TDA_RA_RX_OBJID);
	obj_tx = op_td_get_int(pkptr,OPC_TDA_RA_TX_OBJID);
	obj_rx = op_topo_in_assoc(obj_rx,0);
	obj_tx = op_topo_out_assoc(obj_tx,0);

	op_ima_obj_attr_get_int32(op_topo_parent(obj_rx),"user id",&rx_id);
	op_ima_obj_attr_get_int32(op_topo_parent(obj_tx),"user id",&tx_id);

	op_ima_obj_attr_get(obj_tx,"target longitude",&tx_gamma);
	op_ima_obj_attr_get(obj_rx,"target longitude",&rx_gamma);

	op_ima_obj_attr_get(obj_tx,"target latitude",&tx_alpha);
	op_ima_obj_attr_get(obj_rx,"target latitude",&rx_alpha);

	op_ima_obj_attr_get(obj_tx,"pattern",tx_pa);
	op_ima_obj_attr_get(obj_rx,"pattern",rx_pa);

	// printf("dra_chanmatch:[%d]->[%d] txgamma = %lf rxgamma = %lf tx_pa = %s rx_pa = %s\n",tx_id,rx_id,tx_gamma,rx_gamma,tx_pa,rx_pa);


		/* Obtain the geocentric coordinates of the transmitter. */
	tx_x = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_X);
	tx_y = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_Y);
	tx_z = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_Z);

	/* Obtain the geocentric coordinates of the receiver. */
	rx_x = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_X);
	rx_y = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_Y);
	rx_z = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_Z);

	
	// printf("%d->%d:dra_chanmatch0:rx_x=%lf,tx_x=%lf,rx_y=%lf,tx_y=%lf,rx_z=%lf,tx_z=%lf\n",tx_id,rx_id,rx_x,tx_x,rx_y,tx_y,rx_z,tx_z);



	////////////////TA 

	/* Compute the vector from the transmitter to the receiver. */
	dif_x = rx_x - tx_x;
	dif_y = rx_y - tx_y;
	dif_z = rx_z - tx_z;

	op_ima_obj_attr_get_dbl(obj_tx,"target latitude",&alpha);
	op_ima_obj_attr_get_dbl(obj_tx,"target longitude",&gamma);

	gamma = VOSC_NA_DEG_TO_RAD*(gamma+180);
    alpha = VOSC_NA_DEG_TO_RAD*alpha;

    D = 10000;
    lambda =  VOSC_NA_DEG_TO_RAD*op_td_get_dbl(pkptr, OPC_TDA_RA_TX_LONG);
    phi = VOSC_NA_DEG_TO_RAD*op_td_get_dbl(pkptr, OPC_TDA_RA_TX_LAT);
    h = op_td_get_dbl(pkptr, OPC_TDA_RA_TX_ALT);

	cos_lambda = cos(lambda);
	sin_lambda = sin(lambda);
	cos_phi = cos(phi);
	sin_phi = sin(phi);
	cos_alpha = cos(alpha);
	sin_alpha = sin(alpha);
	cos_gamma = cos(gamma);
	sin_gamma = sin(gamma);


	y1 = D*cos_lambda*cos_phi*sin_alpha - D*cos_alpha*sin_gamma*sin_lambda - D*cos_alpha*cos_gamma*cos_lambda*sin_phi;
 	y2 = D*cos_alpha*cos_lambda*sin_gamma + D*cos_phi*sin_alpha*sin_lambda - D*cos_alpha*cos_gamma*sin_lambda*sin_phi;
    y3 = D*sin_alpha*sin_phi + D*cos_alpha*cos_gamma*cos_phi;

    y = D;
	dif_mag = sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);

	if (dif_mag > 30e3)
	{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
		// printf("dra_chanmatch0:%lf out of comm range\n",dif_mag);
		FOUT
	}

	/* Setup the angles at which to lookup gain.				*/
	/* In the rotated coordinate system, these are really		*/
	/* just the angles of the transmission vector. However,		*/
	/* note that here again the difference in the coordinate 	*/
	/* systems of the antenna and the geocentric axes is		*/
	/* accomodated for by modiftying the phi angle.				*/
	arg_phi = VOSC_NA_RAD_TO_DEG*acos((y1*dif_x+y2*dif_y+y3*dif_z)/(y*dif_mag));
	op_ima_obj_attr_get_dbl(obj_rx,"pointing ref. phi",&beam_width);
	// printf("[%.6f][%d]<-[%d]dra_chanmatch:tx phi = %3.3lf, bw = %3.3lf\n",op_sim_time(),rx_id,tx_id,arg_phi,beam_width);
	if(arg_phi > beam_width/2)
	{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
		FOUT
	}
	else
		printf("@%f dra_chanmatch:[%d]->[%d] recv,arg_phi=%f\n",op_sim_time(),tx_id,rx_id,arg_phi);



	///////////////////////RA

		/* Compute the vector from the receiver to the transmitter. */
	dif_x = tx_x - rx_x;
	dif_y = tx_y - rx_y;
	dif_z = tx_z - rx_z;

	op_ima_obj_attr_get_dbl(obj_rx,"target latitude",&alpha);
	op_ima_obj_attr_get_dbl(obj_rx,"target longitude",&gamma);

	gamma = VOSC_NA_DEG_TO_RAD*(gamma+180);
    alpha = VOSC_NA_DEG_TO_RAD*alpha;

    D = 10000;
    lambda =  VOSC_NA_DEG_TO_RAD*op_td_get_dbl(pkptr, OPC_TDA_RA_RX_LONG);
    phi = VOSC_NA_DEG_TO_RAD*op_td_get_dbl(pkptr, OPC_TDA_RA_RX_LAT);
    h = op_td_get_dbl(pkptr, OPC_TDA_RA_RX_ALT);

	cos_lambda = cos(lambda);
	sin_lambda = sin(lambda);
	cos_phi = cos(phi);
	sin_phi = sin(phi);
	cos_alpha = cos(alpha);
	sin_alpha = sin(alpha);
	cos_gamma = cos(gamma);
	sin_gamma = sin(gamma);

	y1 = D*cos_lambda*cos_phi*sin_alpha - D*cos_alpha*sin_gamma*sin_lambda - D*cos_alpha*cos_gamma*cos_lambda*sin_phi;
 	y2 = D*cos_alpha*cos_lambda*sin_gamma + D*cos_phi*sin_alpha*sin_lambda - D*cos_alpha*cos_gamma*sin_lambda*sin_phi;
    y3 = D*sin_alpha*sin_phi + D*cos_alpha*cos_gamma*cos_phi;

    y = D;
	dif_mag = sqrt(dif_x*dif_x+dif_y*dif_y+dif_z*dif_z);


	arg_phi = VOSC_NA_RAD_TO_DEG*acos((y1*dif_x+y2*dif_y+y3*dif_z)/(y*dif_mag));
	
	// printf("dra_chanmatch:rx phi = %3.3lf, bw = %3.3lf\n",arg_phi,beam_width);
	

	// if(arg_phi > beam_width/2)
	// {
	// 	op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
	// 	FOUT
	// }
	// else
	// {
	// 	printf("dra_chanmatch:[%d](%3.1lf,%3.1lf)<-[%d](%3.1lf,%3.1lf) arg_phi = %3.1lf bw = %3.1lf\n",rx_id,rx_gamma,rx_alpha,tx_id,tx_gamma,tx_alpha,arg_phi,beam_width);
	// }
	


	/* For non-overlapping bands, the packet has no	*/
	/* effect; such packets are ignored entirely.	*/
	if ((tx_freq > rx_freq + rx_bw) || (tx_freq + tx_bw < rx_freq))
		{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
		FOUT
		}

	/* Otherwise check for channel attribute mismatches which would	*/
	/* cause the in-band packet to be considered as noise.			*/
	if ((tx_freq != rx_freq) || (tx_bw != rx_bw) || 
		(tx_drate != rx_drate) || (tx_code != rx_code) || (tx_mod != rx_mod))
		{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_NOISE);
		FOUT
		}

	/* Otherwise the packet is considered a valid transmission which	*/
	/* could eventually be accepted at the error correction stage.		*/
	op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_VALID);

	FOUT
	}                

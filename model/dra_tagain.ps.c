/* dra_tagain.ps.c */                                                       
/* Default transmitter antenna gain model for radio link Transceiver Pipeline */

/****************************************/
/*  	  Copyright (c) 1993-2008		*/
/*      by OPNET Technologies, Inc.     */
/*       (A Delaware Corporation)       */
/*    7255 Woodmont Av., Suite 250      */
/*     Bethesda, MD 20814, U.S.A.       */
/*         All Rights Reserved.         */
/****************************************/

#include "opnet.h"
#include <math.h>

#if defined (__cplusplus)
extern "C"
#endif
void
dra_tagain_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
    {
	double		tx_x, tx_y, tx_z;
	double		rx_x, rx_y, rx_z;
	double		dif_x, dif_y, dif_z, dist_xy;
	double		rot1_x, rot1_y, rot1_z;
	double		rot2_x, rot2_y, rot2_z;
	double		rot3_x, rot3_y, rot3_z;
	double		rx_phi, rx_theta, point_phi, point_theta;
	double		bore_phi, bore_theta, lookup_phi, lookup_theta, gain;
	double		alpha,gamma,phi,lambda,y,y1,y2,y3,D,h,dif_mag;
	double		cos_lambda,sin_lambda,cos_phi,sin_phi,cos_alpha,sin_alpha,cos_gamma,sin_gamma;
	Objid		obj_tx;
	Vartype		pattern_table;

	/** Compute the gain associated with the transmitter's antenna. **/
	FIN_MT (dra_tagain (pkptr));

	/* Obtain handle on receiving antenna's gain. */
	pattern_table = op_td_get_ptr (pkptr, OPC_TDA_RA_TX_PATTERN);

	/* Special case: By convention a nil table address indicates an 	*/
	/* isotropic antenna pattern. Thus no calculations are necessary.	*/
	if (pattern_table == OPC_NIL)
		{
		/* Assign zero dB gain regardless of transmission direction. */
		op_td_set_dbl (pkptr, OPC_TDA_RA_TX_GAIN, 0.0);
		FOUT
		}

	/* Obtain the geocentric coordinates of the transmitter. */
	tx_x = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_X);
	tx_y = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_Y);
	tx_z = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_GEO_Z);

	/* Obtain the geocentric coordinates of the receiver. */
	rx_x = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_X);
	rx_y = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_Y);
	rx_z = op_td_get_dbl (pkptr, OPC_TDA_RA_RX_GEO_Z);

	/* Compute the vector from the transmitter to the receiver. */
	dif_x = rx_x - tx_x;
	dif_y = rx_y - tx_y;
	dif_z = rx_z - tx_z;

	/* Special case: If transmitter and receiver are the same  */
	/* then calculations are unnecessary.  We set gain = 0     */
	if ((dif_x == 0) && (dif_y == 0) && (dif_z == 0))
		{
		op_td_set_dbl(pkptr, OPC_TDA_RA_TX_GAIN, 0.0);
		FOUT;
		}

	/* Determine phi, theta pointing directions for antenna.		*/
	/* These are computed based on the target point of the antenna	*/
	/* module and the position of the transmitter.					*/
	point_phi = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_PHI_POINT);
	point_theta = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_THETA_POINT);

	/* Determine antenna pointing reference direction				*/
	/* (usually boresight cell of pattern).							*/
	/* Note that the difference in selected coordinate systems		*/
	/* between the antenna definiton and the geocentric axes,		*/
	/* is accomodated for here by modifying the given phi value.	*/
	bore_phi = 90.0 - op_td_get_dbl (pkptr, OPC_TDA_RA_TX_BORESIGHT_PHI);
	bore_theta = op_td_get_dbl (pkptr, OPC_TDA_RA_TX_BORESIGHT_THETA);


	obj_tx = op_td_get_int(pkptr,OPC_TDA_RA_TX_OBJID);
	obj_tx = op_topo_out_assoc(obj_tx,0);

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

	/* Setup the angles at which to lookup gain.				*/
	/* In the rotated coordinate system, these are really		*/
	/* just the angles of the transmission vector. However,		*/
	/* note that here again the difference in the coordinate 	*/
	/* systems of the antenna and the geocentric axes is		*/
	/* accomodated for by modiftying the phi angle.				*/
	lookup_phi = VOSC_NA_RAD_TO_DEG*acos((y1*dif_x+y2*dif_y+y3*dif_z)/(y*dif_mag));
	lookup_theta = 0;

	/* Obtain gain of antenna pattern at given angles. */
	gain = op_tbl_pat_gain (pattern_table, lookup_phi, lookup_theta);

	// printf("dra_tagain:lookup_phi = %f gain = %f\n",lookup_phi,gain);
	/* Set the tx antenna gain in the packet's transmission data attribute. */
	op_td_set_dbl (pkptr, OPC_TDA_RA_TX_GAIN, gain);

	FOUT;
	}

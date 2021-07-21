#include <math.h>
#include "kalman.h"

float _err_measure_GP2Y, _err_measure_eCO2, _err_measure_eTVOC;
float _err_estimate_GP2Y, _err_estimate_eCO2, _err_estimate_eTVOC;
float _q_GP2Y, _q_eCO2, _q_eTVOC;
float _current_estimate_GP2Y, _current_estimate_eCO2, _current_estimate_eTVOC;
float _last_estimate_GP2Y, _last_estimate_eCO2, _last_estimate_eTVOC;
float _kalman_gain_GP2Y, _kalman_gain_eCO2, _kalman_gain_eTVOC;

//Kalman filter for dust sensor
float KalmanFilter_GP2Y(float mea_e_GP2Y, float est_e_GP2Y, float q_GP2Y){
	_err_measure_GP2Y = mea_e_GP2Y;
	_err_estimate_GP2Y = est_e_GP2Y;
	_q_GP2Y = q_GP2Y;
}

float updateEstimate_GP2Y(float mea_GP2Y){
	_kalman_gain_GP2Y = _err_estimate_GP2Y/(_err_estimate_GP2Y + _err_measure_GP2Y);
	_current_estimate_GP2Y = _last_estimate_GP2Y + _kalman_gain_GP2Y*(mea_GP2Y - _last_estimate_GP2Y);
	_err_estimate_GP2Y = (1.0 - _kalman_gain_GP2Y)*_err_estimate_GP2Y + fabs(_last_estimate_GP2Y - _current_estimate_GP2Y)*_q_GP2Y;
	_last_estimate_GP2Y = _current_estimate_GP2Y;
	
	return _current_estimate_GP2Y;
}

void setMeasurementError_GP2Y(float mea_e_GP2Y)
{
	_err_measure_GP2Y = mea_e_GP2Y;
}

void setEstimateError_GP2Y(float est_e_GP2Y)
{
	_err_estimate_GP2Y = est_e_GP2Y;
}

void setProcessNoise_GP2Y(float q_GP2Y)
{
	_q_GP2Y = q_GP2Y;
}

float getKalmanGain_GP2Y()
{
	return _kalman_gain_GP2Y;
}

float getEstimateError_GP2Y()
{
	return _err_estimate_GP2Y;
}

//Kalman filter for eCO2
float KalmanFilter_eCO2(float mea_e_eCO2, float est_e_eCO2, float q_eCO2){
	_err_measure_eCO2 = mea_e_eCO2;
	_err_estimate_eCO2 = est_e_eCO2;
	_q_eCO2 = q_eCO2;
}

float updateEstimate_eCO2(float mea_eCO2){
	_kalman_gain_eCO2 = _err_estimate_eCO2/(_err_estimate_eCO2 + _err_measure_eCO2);
	_current_estimate_eCO2 = _last_estimate_eCO2 + _kalman_gain_eCO2*(mea_eCO2 - _last_estimate_eCO2);
	_err_estimate_eCO2 = (1.0 - _kalman_gain_eCO2)*_err_estimate_eCO2 + fabs(_last_estimate_eCO2 - _current_estimate_eCO2)*_q_eCO2;
	_last_estimate_GP2Y = _current_estimate_GP2Y;
	
	return _current_estimate_eCO2;
}

void setMeasurementError_eCO2(float mea_e_eCO2)
{
	_err_measure_eCO2 = mea_e_eCO2;
}

void setEstimateError_eCO2(float est_e_eCO2)
{
	_err_estimate_eCO2 = est_e_eCO2;
}

void setProcessNoise_eCO2(float q_eCO2)
{
	_q_eCO2 = q_eCO2;
}

float getKalmanGain_eCO2()
{
	return _kalman_gain_eCO2;
}

float getEstimateError_eCO2()
{
	return _err_estimate_eCO2;
}

//Kalman filter for eTVOC
float KalmanFilter_eTVOC(float mea_e_eTVOC, float est_e_eTVOC, float q_eTVOC){
	_err_measure_eTVOC = mea_e_eTVOC;
	_err_estimate_eTVOC = est_e_eTVOC;
	_q_eTVOC = q_eTVOC;
}

float updateEstimate_eTVOC(float mea_eTVOC){
	_kalman_gain_eTVOC = _err_estimate_eTVOC/(_err_estimate_eTVOC + _err_measure_eTVOC);
	_current_estimate_eTVOC = _last_estimate_eTVOC + _kalman_gain_eTVOC*(mea_eTVOC - _last_estimate_eTVOC);
	_err_estimate_eTVOC = (1.0 - _kalman_gain_eTVOC)*_err_estimate_eTVOC + fabs(_last_estimate_eTVOC - _current_estimate_eTVOC)*_q_eTVOC;
	_last_estimate_eTVOC = _current_estimate_eTVOC;
	
	return _current_estimate_eTVOC;
}

void setMeasurementError_eTVOC(float mea_e_eTVOC)
{
	_err_measure_eTVOC = mea_e_eTVOC;
}

void setEstimateError_eTVOC(float est_e_eTVOC)
{
	_err_estimate_eTVOC = est_e_eTVOC;
}

void setProcessNoise_eTVOC(float q_eTVOC)
{
	_q_eTVOC = q_eTVOC;
}

float getKalmanGain_eTVOC()
{
	return _kalman_gain_eTVOC;
}

float getEstimateError_eTVOC()
{
	return _err_estimate_eTVOC;
}
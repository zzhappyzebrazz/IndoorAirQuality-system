#ifndef kalman_h
#define kalman_h

//dust sensor
float KalmanFilter_GP2Y(float mea_e_GP2Y, float est_e_GP2Y, float q_GP2Y);
float updateEstimate_GP2Y(float mea_GP2Y);
void setMeasurementError_GP2Y(float mea_e_GP2Y);
void setEstimateError_GP2Y(float est_e_GP2Y);
void setProcessNoise_GP2Y(float q_GP2Y);
float getKalmanGain_GP2Y(void);
float getEstimateError_GP2Y(void);

//eCO2
float KalmanFilter_eCO2(float mea_e_eCO2, float est_e_eCO2, float q_eCO2);
float updateEstimate_eCO2(float mea_eCO2);
void setMeasurementError_eCO2Y(float mea_e_eCO2);
void setEstimateError_eCO2(float est_e_eCO2);
void setProcessNoise_eCO2(float q_eCO2);
float getKalmanGain_eCO2(void);
float getEstimateError_eCO2(void);

//eTVOC
float KalmanFilter_eTVOC(float mea_e_eTVOC, float est_e_eTVOC, float q_eTVOC);
float updateEstimate_eTVOC(float mea_eTVOC);
void setMeasurementError_eTVOC(float mea_e_eTVOC);
void setEstimateError_eTVOC(float est_e_eTVOC);
void setProcessNoise_eTVOC(float q_eTVOC);
float getKalmanGain_eTVOC(void);
float getEstimateError_eTVOC(void);
#endif

#include "glass.h"
#include "openvr_driver.h"
#include "headset_control.h"

using namespace vr;

static GlassEvent EventCallback;
static GlassHandle glass;


class NativeGlass: public HeadsetControl
{
public:
	virtual bool connect(void) override {

		_q = &qbuf[0];
		_q->w = 1;
		_q->x = 0;
		_q->y = 0;
		_q->z = 0;
		return true;
	}

	virtual bool setDisplayMode(int mode) override {
		return true;
	}

	virtual HmdQuaternion_t getPose(void) override {
		//return simulation();
		return *_q;
	}

	void setPose(HmdQuaternion_t q) {
		if (_q == &qbuf[0]) {
			qbuf[1] = q;
			InterlockedExchangePointer((volatile PVOID *)&_q, &qbuf[1]);
		}
		else {
			qbuf[0] = q;
			InterlockedExchangePointer((volatile PVOID*)&_q, &qbuf[0]);
		}
	}

private:
	HmdQuaternion_t *_q;
	HmdQuaternion_t qbuf[2];

	HmdQuaternion_t ToQuaternion(double yaw, double pitch, double roll) // yaw (Z), pitch (Y), roll (X)
	{
		// Abbreviations for the various angular functions
		double cy = cos(yaw * 0.5);
		double sy = sin(yaw * 0.5);
		double cp = cos(pitch * 0.5);
		double sp = sin(pitch * 0.5);
		double cr = cos(roll * 0.5);
		double sr = sin(roll * 0.5);

		HmdQuaternion_t q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;

		return q;
	}

	HmdQuaternion_t simulation(void) {
		static double yaw = 0, pitch = 0, roll = 0;
		static int count = 0;
		yaw += 0.02 / 180.f * 3.1415926;

		return ToQuaternion(yaw, pitch, roll);
	}

};

static NativeGlass* nativeGlassInstance = NULL;

static  void QuaternionMultiply(float *var0, float* var1, float* result) {
	float temp[4] = {0};
	float var2 = var0[0];
	float var3 = var0[1];
	float var4 = var0[2];
	float var5 = var0[3];
	float var6 = var1[0];
	float var7 = var1[1];
	float var8 = var1[2];
	float var9 = var1[3];
	//x
	result[0] = (var5 * var6 + var2 * var9 + var3 * var8 - var4 * var7);
	//y
	result[1] = (var5 * var7 - var2 * var8 + var3 * var9 + var4 * var6);
	//z
	result[2] = (var5 * var8 + var2 * var7 - var3 * var6 + var4 * var9);
	//w
	result[3] = var5 * var9 - var2 * var6 - var3 * var7 - var4 * var8;
}

static void BBToUnityTranslation(float* src, float* result) {
	//x 90
	float rotation4Unity[4] = {  -0.707f, 0, 0, 0.707f};
	//y 90
	// float[] rotation4Unity = new float[ ]{  0, 0.707f, 0, 0.707f};
	//z 90
	// float[] rotation4Unity = new float[ ]{ 0, 0, 0.707f, 0.707f};
	float temp[4];
	QuaternionMultiply(src, rotation4Unity, temp);

	result[0] = -temp[1];
	result[1] = temp[3];
	result[2] = -temp[0];
	result[3] = -temp[2];
	return;
};

void onGameRotationVectorEvent(unsigned long timeStamp, float* quaternion) {
	//TODO
	HmdQuaternion_t q;
	float temp[4];
	BBToUnityTranslation(quaternion, temp);

	q.x = temp[0];
	q.y = temp[1];
	q.z = temp[2];
	q.w = temp[3];

	if (nativeGlassInstance)
		nativeGlassInstance->setPose(q);

}

void onRawSensorEvent(unsigned long timeStamp, int id, float* data, int status) {
	//TODO
	if (id == MAGNETIC) {

	}
	else if (id == ACCELEROMETER) {
		
	}
	else if (id == GYROSCOPE) {

	}
}



HeadsetControl* getNativeGlassInstance(void) {
	if (nativeGlassInstance == NULL) {
		memset(&EventCallback, 0, sizeof(EventCallback));
		EventCallback.onGameRotationVectorEvent = onGameRotationVectorEvent;
		EventCallback.onRawSensorEvent = onRawSensorEvent;
		glass = GlassInitial(&EventCallback);
		GlassOpen(glass);
		nativeGlassInstance = new NativeGlass;
		nativeGlassInstance->connect();
	}

	return (HeadsetControl*)nativeGlassInstance;
}
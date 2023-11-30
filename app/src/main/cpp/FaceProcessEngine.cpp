//
// Created by Lenovo on 2023/5/10.
//

#include "FaceProcessEngine.h"
#include "dlib/opencv/cv_image.h"
#include <dlib/image_loader/load_image.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
FaceProcessEngine *engine;

void FaceProcessEngine::convertColor(cv::Mat *pSrcRGBMat, cv::Mat *pDstGrayMat) {
    cv::cvtColor(*pSrcRGBMat,*pDstGrayMat,cv::COLOR_BGR2GRAY);
}

FaceProcessEngine::FaceProcessEngine(std::istream& model,std::istream& net) {
    this->m_detector = get_frontal_face_detector();
    //导入模型
    deserialize(this->m_sp,model);
    deserialize(this->m_net,net);

}

void FaceProcessEngine::LoadImage(array2d<rgb_pixel> &image) {
    this->FaceImage.swap(image);
    LOGD("load image");
}

void FaceProcessEngine::LoadImage(cv::Mat *pSrcRGBMat) {
    //cv::Mat faceMat = *pSrcRGBMat;
    //cv_image<rgb_pixel> dlib_img(*pSrcRGBMat);
    //m_FaceMat = cv::imread("/sdcard/header.png",cv::IMREAD_UNCHANGED);
    LOGD("load image begin");

    this->CopyImage(pSrcRGBMat);
    //load_image(this->FaceImage,"/sdcard/header.png");
    LOGD("load image end");
}

const std::vector<full_object_detection> & FaceProcessEngine::FaceDetect() {
    std::vector<rectangle> dets = std::move(this->m_detector(this->FaceImage));
    LOGD("human faces num is %d\n",dets.size());
    m_faceNum = dets.size();
    m_faces.clear();
    m_faceShapes.clear();
    for(auto &det : dets) {
        cv::Rect r;
        r.x = det.left()/engine->getX();
        r.y = det.top()/engine->getY();
        r.width = det.width()/engine->getX();
        r.height = det.height()/engine->getY();
        cv::rectangle(this->m_FaceMat,r,cv::Scalar(0,0,255),1,1,0);
        full_object_detection shape = this->m_sp(this->FaceImage,det);
        this->m_faceShapes.push_back(shape);
        dlib::matrix<dlib::rgb_pixel> face_chip;
        dlib::extract_image_chip(FaceImage, dlib::get_face_chip_details(shape,150,0.25), face_chip);
        m_faces.push_back(std::move(face_chip));
    }
    this->m_faceDescriptors = m_net(m_faces);
//    for(auto &des : m_faceDescriptors) {
//        LOGD("face des is :");
//        for(auto &val : des) {
//            LOGD("%f ",val);
//        }
//
//    }
    return this->m_faceShapes;
}

void FaceProcessEngine::LoadImage(const string &filename) {
    load_image(this->FaceImage,filename);
}

void FaceProcessEngine::CopyImage(cv::Mat *faceimage) {
    cv::Mat dst;
    this->m_FaceMat = (*faceimage).clone();
    //this->m_FaceMat = *faceimage;
    cv::cvtColor(*faceimage, dst, CV_BGR2GRAY);

    dlib::assign_image( this->FaceImage, dlib::cv_image<uchar>(dst));
//    this->FaceImage.set_size(faceimage->rows,faceimage->cols);
//    for(int i = 0; i < faceimage->rows; ++i) {
//        for(int j = 0; j < faceimage->cols; ++j) {
//            this->FaceImage[i][j].blue = faceimage->at<cv::Vec3b>(i,j)[0];
//            this->FaceImage[i][j].green = faceimage->at<cv::Vec3b>(i,j)[1];
//            this->FaceImage[i][j].red = faceimage->at<cv::Vec3b>(i,j)[2];
//        }
//    }
    const rectangle &rectOri = get_rect(FaceImage);
    pyramid_up(FaceImage);
    const rectangle &rectUp = get_rect(FaceImage);
    scalY = rectUp.bottom() / rectOri.bottom();
    scalX = rectUp.right() / rectOri.right();
}

cv::Mat &FaceProcessEngine::getFaceImage() {
    return m_FaceMat;
}

std::vector<full_object_detection> &FaceProcessEngine::getFaceShapes() {
    return m_faceShapes;
}

std::vector<dlib::matrix<float,0,1>> &FaceProcessEngine::getFaceDescriptors() {
    return this->m_faceDescriptors;
}

int FaceProcessEngine::detectFaceNum(array2d<rgb_pixel> &image) {
    std::vector<rectangle> dets = std::move(m_detector(image));
    return dets.size();
}

int FaceProcessEngine::getFaceNum() {
    return m_faceNum;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_faceidentification_MainActivity_convertColor(JNIEnv *env, jobject thiz,
                                                              jlong src_rgbmat_addr,
                                                              jlong dst_gray_mat_addr) {
    __android_log_print(ANDROID_LOG_DEBUG, "JPEG_JNI", "convert..........");
    //std::cout<<"convert.................."<<std::endl;
    cv::Mat *pSrcRGBMat = (cv::Mat*)src_rgbmat_addr;
    cv::Mat *pDstGrayMat = (cv::Mat*)dst_gray_mat_addr;
    FaceProcessEngine::convertColor(pSrcRGBMat,pDstGrayMat);
}
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_faceidentification_MainActivity_faceDetect(JNIEnv *env, jobject thiz,
                                                            jlong src_face_addr) {
    cv::Mat *srcFaceMat = (cv::Mat*)src_face_addr;
    engine->LoadImage(srcFaceMat);
    std::vector<full_object_detection> points =  engine->FaceDetect();
    //jintArray intArray = env->NewIntArray(384*250);
    //cv::Mat temp;
    //temp=cv::imread("/sdcard/header.png",cv::IMREAD_UNCHANGED);
    LOGD("image : [%d =========== %d]",engine->getFaceImage().rows,engine->getFaceImage().cols);
    jintArray intArray = env->NewIntArray(engine->getFaceImage().cols*engine->getFaceImage().rows);

    for(const auto &point : points) {
        //cv::rectangle(temp, cv::Rect(point., point.part(i).x(), face.pos.width, face.pos.height), CV_RGB(128, 128, 255), 3);
        for(int i = 0; i < 68; ++i) {
            //下巴到脸颊 0 ~ 16
            //左边眉毛 17 ~ 21
            //右边眉毛 21 ~ 26
            //鼻梁     27 ~ 30
            //鼻孔        31 ~ 35
            //左眼        36 ~ 41
            //右眼        42 ~ 47
            //嘴唇外圈  48 ~ 59
            //嘴唇内圈  59 ~ 67
            //LOGD("[%d === %d] ", point.part(i).x(),point.part(i).y());
            cv::circle(engine->getFaceImage(),cvPoint(static_cast<int>(point.part(i).x()) / engine->getX(),
                    static_cast<int>(point.part(i).y()) / engine->getY()),
                       1, cv::Scalar(0, 0, 255,255),
                       2,cv::LINE_AA, 0);
        }
    }
    const  jint * buf= reinterpret_cast<const jint *>(engine->getFaceImage().data);
    if(buf == nullptr) {
        LOGD("error,buf is nullptr\n");
    }else {
        env->SetIntArrayRegion(intArray, 0, engine->getFaceImage().cols*engine->getFaceImage().rows, buf);
    }
    return intArray;

}
struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};
std::istream& getAssetFile(AAssetManager* native_asset,const char* filename) {
    AAsset *assetFile = AAssetManager_open(native_asset, filename, AASSET_MODE_BUFFER);
    //get file length
    size_t file_length = static_cast<size_t>(AAsset_getLength(assetFile));
    char *model_buffer = (char *) malloc(file_length);
    //read file data
    AAsset_read(assetFile, model_buffer, file_length);
    //the data has been copied to model_buffer, so , close it
    AAsset_close(assetFile);
    membuf mem_buf(model_buffer, model_buffer + file_length);
    std::istream in(&mem_buf);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_faceidentification_MainActivity_initEngine(JNIEnv *env, jobject thiz,jobject assetManager) {
    const char *file_shape_name = "shape_predictor_68_face_landmarks.dat";
    const char *file_net_name = "dlib_face_recognition_resnet_model_v1.dat";
    //get AAssetManager
    AAssetManager *native_asset = AAssetManager_fromJava(env, assetManager);

    //open file
    AAsset *assetFile = AAssetManager_open(native_asset, file_shape_name, AASSET_MODE_BUFFER);
    //get file length
    size_t file_length = static_cast<size_t>(AAsset_getLength(assetFile));
    char *model_buffer = (char *) malloc(file_length);
    //read file data
    AAsset_read(assetFile, model_buffer, file_length);
    //the data has been copied to model_buffer, so , close it
    AAsset_close(assetFile);
    membuf mem_buf(model_buffer, model_buffer + file_length);
    std::istream shapeIn(&mem_buf);

    assetFile = AAssetManager_open(native_asset, file_net_name, AASSET_MODE_BUFFER);
    //get file length
    file_length = static_cast<size_t>(AAsset_getLength(assetFile));
    char* net_buffer = (char *) malloc(file_length);
    //read file data
    AAsset_read(assetFile, net_buffer, file_length);
    //the data has been copied to model_buffer, so , close it
    AAsset_close(assetFile);
    membuf mem_bf(net_buffer, net_buffer + file_length);
    std::istream netIn(&mem_bf);
    engine = new FaceProcessEngine(shapeIn,netIn);
    free(model_buffer);
    free(net_buffer);

}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_faceidentification_MainActivity_getFaceShapes(JNIEnv *env,jobject thiz) {
    jobjectArray faceFeatures = NULL;
    jclass landMark = env->FindClass("com/example/faceidentification/face/FaceShapes");
    jmethodID landMark_construct = env->GetMethodID(landMark, "<init>", "(II)V");
    std::vector<full_object_detection> &featurePoints = engine->getFaceShapes();
    jobjectArray shape = env->NewObjectArray(68,landMark, nullptr);
    faceFeatures = env->NewObjectArray(featurePoints.size(),env->GetObjectClass(shape), nullptr);
    env->DeleteLocalRef(shape);
    int index = 0;
    for(auto const &featurePointer : featurePoints) {
        jobjectArray pointers = env->NewObjectArray(68, landMark, NULL);
        for(int i = 0; i < 68; ++i) {
            int x = featurePointer.part(i).x();
            int y = featurePointer.part(i).y();
            jobject pointer = env->NewObject(landMark, landMark_construct, x, y);
            env->SetObjectArrayElement(pointers, i, pointer);
            env->DeleteLocalRef(pointer);
        }
        env->SetObjectArrayElement(faceFeatures, index, pointers);
        env->DeleteLocalRef(pointers);
        index++;
    }
    return faceFeatures;
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_example_faceidentification_MainActivity_getFacesDescriptors(JNIEnv *env,jobject thiz) {
    std::vector<dlib::matrix<float,0,1>> &des = engine->getFaceDescriptors();
    jclass doubleArr = env->FindClass("[D");
    jobjectArray faceDescribs = env->NewObjectArray(des.size(), doubleArr , nullptr);
    jdouble temp[128];
    for(int i = 0; i < des.size(); ++i) {
        jdoubleArray colArr = env->NewDoubleArray(128);
        int j = 0;
        for(auto &val :  des[i]) {
            temp[j] = val;
            //LOGD("%d = %f",j,val);
            j++;
        }
        env->SetDoubleArrayRegion(colArr,0,128,temp);
        env->SetObjectArrayElement(faceDescribs, i, colArr);
        env->DeleteLocalRef(colArr);
    }
    return faceDescribs;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_faceidentification_FaceRecordActivity_detectFaceNum(JNIEnv *env,jobject thiz,jlong src_face_addr) {
    cv::Mat *srcFaceMat = (cv::Mat*)src_face_addr;
    cv::Mat dst;
    cv::cvtColor(*srcFaceMat, dst, CV_BGR2GRAY);
    array2d<rgb_pixel> image;
    dlib::assign_image( image, dlib::cv_image<uchar>(dst));
    int num = engine->detectFaceNum(image);
    return num;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_faceidentification_MainActivity_getFaceNum(JNIEnv *env,jobject thiz) {
    int num = engine->getFaceNum();
    return num;
}
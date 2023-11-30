//
// Created by Lenovo on 2023/5/10.
//

#ifndef FACEIDENTIFICATION_FACEPROCESSENGINE_H
#define FACEIDENTIFICATION_FACEPROCESSENGINE_H
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <android/log.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/dnn.h>
#include <dlib/image_loader/image_loader.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/graph_utils.h>
using namespace dlib;
using namespace std;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

template <long num_filters, typename SUBNET> using con5d = con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = con<num_filters,5,5,1,1,SUBNET>;

template <typename SUBNET> using downsampler  = relu<affine<con5d<32, relu<affine<con5d<32, relu<affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5  = relu<affine<con5<45,SUBNET>>>;

using cnn_anet_type = loss_mmod<con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                                             alevel0<
                                                     alevel1<
                                                             alevel2<
                                                                     alevel3<
                                                                             alevel4<
                                                                                     max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                                                                                     input_rgb_image_sized<150>
                                                                     >>>>>>>>>>>>;
#include <android/log.h>
#define LOG_TAG "FaceDetection/DetectionBasedTracker"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
class FaceProcessEngine {
public:
    FaceProcessEngine(std::istream& model,std::istream& net);
    void LoadImage(array2d<rgb_pixel> &image);
    void LoadImage(cv::Mat *pSrcRGBMat);
    void CopyImage(cv::Mat *FaceImage);
    void LoadImage(const string &filename);
    float getX(){return scalX;};
    float getY(){return scalY;};
    const std::vector<full_object_detection>& FaceDetect();
    int detectFaceNum(array2d<rgb_pixel> &image);
    int getFaceNum();
    std::vector<full_object_detection>& getFaceShapes();
    std::vector<dlib::matrix<float,0,1>>& getFaceDescriptors();
    cv::Mat& getFaceImage();
    static void convertColor(cv::Mat *pSrcRGBMat,cv::Mat *pDstGrayMat);
private:
    frontal_face_detector m_detector;
    cv::Mat m_FaceMat;
    shape_predictor m_sp;
    anet_type m_net;
    int m_faceNum;
    array2d<rgb_pixel> FaceImage;
    std::vector<dlib::matrix<dlib::rgb_pixel>> m_faces;
    std::vector<full_object_detection> m_faceShapes;
    std::vector<dlib::matrix<float,0,1>> m_faceDescriptors;
    float scalX,scalY;
};



#endif //FACEIDENTIFICATION_FACEPROCESSENGINE_H

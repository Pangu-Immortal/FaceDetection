# FaceDetection

FaceDetection 是在安卓端运行的人脸识别APP，基于opencv以及dlib实现，可进行人脸68点检测，提取人脸特征。

## 版本

- OpenCV 4.8.1 的安卓 SDK

- Dlib 19.10 

## 特征

FaceDetection 具有以下特性：

- 检测人脸
- 工作在安卓端
- 支持多张人脸检测


## 说明

- 项目依赖dlib_face_recognition_resnet_model_v1.dat与shape_predictor_68_face_landmarks.dat模型均存放在assets目录。

- 目前 OpenCV 图片加载和提取人脸，特征标记、识别都是使用 Dlib。

- 经过测试 Dlib 的单张人脸特征在三星s10上提取耗时为350ms左右。





#ifndef _GSOC_SFM_CAMERA_H
#define _GSOC_SFM_CAMERA_H 1

#include "macro.h" //SFM_EXPORTS

#include "opencv2/core/core.hpp"
#include <vector>

namespace OpencvSfM{
  class PointOfView;//We will need this class, but PointOfView need our class too...


  /*! \brief This class represent the physical device which take the pictures. 
  *      It is not related to a 3D position which is the role of the PointOfView class.
  *      The role of the class is to store only device related informations like intra parameters, radial and tangential distotion.
  *      This abstract class is not related to a type of camera ( fish eyes... )
  *
  * This class can be used to store device related informations like intra parameters, radial and tangential distortion.
  * If we use the so-called pinhole camera model, a scene view is formed by projecting 3D points into the image plane using a perspective transformation.
  * Usual notation says that a point [ u,v ] from an image is related to the point [ X,Y,Z ] using the following notation :
  * \f[
  * s  \begin{bmatrix} u \\ v \\ 1 \end{bmatrix} =  \begin{bmatrix}f_x & 0 & c_x \\ 0 & f_y & c_y \\ 0 & 0 & 1 \end{bmatrix}
  * \begin{bmatrix} r_{11} & r_{12} & r_{13} & t_1  \\ r_{21} & r_{22} & r_{23} & t_2  \\ r_{31} & r_{32} & r_{33} & t_3 \end{bmatrix}
  * \begin{bmatrix} X \\ Y \\ Z \\ 1  \end{bmatrix}
  * \f]
  *
  * This leads to the following relation between local coordinates and global ones:
  * \f[
  * \begin{array}{l} \vspace{10pt}
  * \begin{bmatrix} x \\ y \\ z \end{bmatrix} = R  \begin{bmatrix} X \\ Y \\ Z \end{bmatrix} + t \\
  * x' = x/z \\ y' = y/z \vspace{10pt} 
  * \end{array}
  * \f]
  * 
  * Additionnal radial and tangeancial distortion are modelized like this:
  * \f[
  * \begin{array}{l}
  * \vspace{10pt} x'' = x'  \dfrac{1 + k_1 r^2 + k_2 r^4 + k_3 r^6}{1 + k_4 r^2 + k_5 r^4 + k_6 r^6} + 2 p_1 x' y' + p_2( r^2 + 2 x'^2 )  \\
  * \vspace{10pt} y'' = y'  \dfrac{1 + k_1 r^2 + k_2 r^4 + k_3 r^6}{1 + k_4 r^2 + k_5 r^4 + k_6 r^6} + p_1 ( r^2 + 2 y'^2 ) + 2 p_2 x' y'  \\
  * \text{where} \quad r^2 = x'^2 + y'^2  \\ u = f_x*x'' + c_x \\ v = f_y*y'' + c_y \end{array}
  * \f]
  * radial_dist_ can be used to store \f$k_1\f$ to \f$k_6\f$
  * tangential_dist_ can be used to store \f$p_1\f$ and \f$p_2\f$
  *
  * So this class is devoted to the conversion between 2D points from pixel image coordinates and 2D points in normalized image coordinates,
  * or ray projection using intra parameters.
  */
  class SFM_EXPORTS Camera
  {
    friend class PointOfView;//PointOfView is a good friend of Camera as it's intimately related!
  protected:

    int img_width;///<Width of the images taken by this camera
    int img_height;///<Height of the images taken by this camera

    std::vector<PointOfView*> pointsOfView_;///<vector of the differents positions of the camera.
    /**
    * As this class is virtual, we can't create a new empty camera...
    * @param img_w width of images the camera produce
    * @param img_h height of images the camera produce
    */
    Camera( int img_w=640, int img_h=480 );
  public:
    virtual ~Camera( void );
    /**
    * use this function to know which parameters are missing
    * @return numbers of parameters needed for this camera
    */
    virtual uchar getNbMissingParams( ) const{return 0;};
    /**
    * use this function to know which parameters are missing
    * @return numbers of parameters needed for this camera
    */
    virtual const std::vector<PointOfView*> getRelatedViews( ) const {
      return pointsOfView_;
    };
    /**
    * get the width of the images taken by this camera
    */
    int getImgWidth() const { return img_width;};
    /**
    * get the height of the images taken by this camera
    */
    int getImgHeight() const { return img_height;};
    /**
    * This method can transform points from image to 3D rays ( homogeneous coordinates )
    */
    virtual std::vector<cv::Vec4d> convertFromImageTo3Dray( std::vector<cv::Vec3d> points ) =0;//we don't know how this transformation can be done, so pure virtual
    
    /**
    * This method can convert 2D points from pixel image coordinates to 2D points in normalized image coordinates
    * @param points 2D points in pixel image homogeneous coordinates.
    * @return 2D points in normalized image homogeneous coordinates.
    */
    virtual std::vector<cv::Vec2d> pixelToNormImageCoordinates(
      std::vector<cv::Vec2d> points ) const=0;
    /**
    * This method can convert 2D points from normalized image coordinates to 2D points in pixel image coordinates
    * @param points 2D points in normalized image homogeneous coordinates.
    * @return 2D points in pixel image coordinates.
    */
    virtual std::vector<cv::Vec2d> normImageToPixelCoordinates(
      std::vector<cv::Vec2d> points ) const=0;
    /**
    * This method return the intra parameters of the camera
    * @return Matrix K of intra parameters
    */
    virtual cv::Mat getIntraMatrix( ) const
    {return cv::Mat::eye( 3,3,CV_64FC1 );};

    /**
    * This method modify the intra parameters of the camera
    * @param values array of intra parameters (order corresponds to SBA)
    //K( 0,0 ) -> values[0]
    //K( 2,0 ) -> values[1],
    //K( 2,1 ) -> values[2],
    //K( 1,1 )/K( 0,0 ) -> values[3]
    //K( 1,0 ) -> values[4]
    * @param nbVal nb values into array...
    * @param add_to_intra if true, the vector is the delta to apply to each intra values
    */
    virtual void updateIntrinsic( double* values, uchar nbVal, bool add_to_intra ) {};
    
    /**
    * This method is useful to get the focal from Intrinsic matrix:
    * @return focal lenght
    */
    virtual double getFocal( ) const=0;
    
    /**
    * Create a new camera from a YAML file.
    * @param node Previously opened YAML file node
    */
    static cv::Ptr<Camera> read( const cv::FileNode& node );
    
    /**
    * Save the camera intra parameters into a YAML file.
    * @param fs Previously opened YAML file node
    */
    virtual void write( cv::FileStorage& fs ) const = 0;

  };

}

#endif
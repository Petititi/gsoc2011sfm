
#include "StructureEstimator.h"
#include "SequenceAnalyzer.h"
#include "PointOfView.h"
#include "PointsToTrack.h"
#include "Camera.h"

namespace OpencvSfM{
  using std::vector;
  using cv::Ptr;

  vector<char> StructureEstimator::computeStructure( unsigned int max_error )
  {
    vector<char> output_mask;
    vector<TrackOfPoints>& tracks = sequence_->getTracks( );
    vector< Ptr< PointsToTrack > > points_to_track = sequence_->getPoints( );

    //for each points:
    vector<TrackOfPoints>::size_type key_size = tracks.size( );
    vector<PointOfView>::size_type num_camera = cameras_->size( );
    int idImage=-1, idPoint=-1;
    vector<TrackOfPoints>::size_type i;

    for ( i=0; i < key_size; i++ )
    {
      TrackOfPoints &track = tracks[ i ];
      unsigned int nviews = track.getNbTrack( );

      CV_DbgAssert( nviews <= cameras_->size( ) );

      cv::Vec3d point_final;
      double distance=track.triangulateRobust( *cameras_,points_to_track, point_final );
      //double distance=track.triangulateLinear( *cameras_,points_to_track, point_final );

      //tests:


      //////////////////////////////////////////////////////////////////////////

      //this is used to take only correct 3D points:
      output_mask.push_back( ( distance<max_error ) );
    }
    return output_mask;
  }

  std::vector< TrackOfPoints > StructureEstimator::computeStructure(
    const std::vector<int>& list_of_images, unsigned int max_error )
  {
    CV_Assert( list_of_images.size( ) > 1 );

    std::vector< TrackOfPoints > points3D;
    vector<TrackOfPoints>& tracks = sequence_->getTracks( );
    vector< Ptr< PointsToTrack > > &points_to_track = sequence_->getPoints( );

    //for each points:
    vector<TrackOfPoints>::size_type key_size = tracks.size( );
    vector<PointOfView>::size_type num_camera = cameras_->size( );
    int idImage=-1, idPoint=-1;
    vector<TrackOfPoints>::size_type i;
    vector<int>::size_type images_size =list_of_images.size( );

    for ( i=0; i < key_size; i++ )
    {
      TrackOfPoints &track = tracks[ i ];
      int nbLinks = 0;
      for( size_t it_img = 0; it_img<images_size ; ++it_img )
        if( track.containImage( list_of_images[ it_img ] ) )
          nbLinks++;

      if( nbLinks > 1 )
      {
        unsigned int nviews = track.getNbTrack( );
        vector<bool> mask;
        for( unsigned int j=0; j<nviews; ++j )
        {
          if( std::find( list_of_images.begin( ),list_of_images.end( ),
            track.getImageIndex( j ) ) != list_of_images.end( ) )
            mask.push_back( true );
          else
            mask.push_back( false );
        }

        cv::Vec3d point_final;
        double distance=track.triangulateRobust( *cameras_,points_to_track, point_final,
          4, mask );

        if( distance<max_error )
          points3D.push_back( track );//only keep correct points
      }

    }
    return points3D;
  }

  void StructureEstimator::removeOutliersTracks( double max_error,
    std::vector< TrackOfPoints >* list_of_tracks )
  {
    vector<char> output_mask;
    if(list_of_tracks==NULL)
      list_of_tracks = &sequence_->getTracks( );
    vector<TrackOfPoints>& tracks = *list_of_tracks;
    vector< Ptr< PointsToTrack > > &points_to_track = sequence_->getPoints( );

    //for each points:
    vector<TrackOfPoints>::size_type key_size = tracks.size( );
    vector<PointOfView>::size_type num_camera = cameras_->size( );
    int idImage=-1, idPoint=-1;
    vector<TrackOfPoints>::size_type i;

    for ( i=0; i < key_size; i++ )
    {
      TrackOfPoints &track = tracks[ i ];
      unsigned int nviews = track.getNbTrack( );

      CV_DbgAssert( nviews <= cameras_->size( ) );

      cv::Ptr<cv::Vec3d> point_final = track.get3DPosition();
      if(point_final.empty())
      {
        cv::Vec3d *pt = new cv::Vec3d;
        track.triangulateRobust( *cameras_,points_to_track, *pt );
        point_final = cv::Ptr<cv::Vec3d>( pt );
      }

      track.removeOutliers( *cameras_, points_to_track, max_error );
    }

  }
}

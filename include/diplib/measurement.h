/*
 * DIPlib 3.0
 * This file contains declarations for measurement-related classes
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_MEASUREMENT_H
#define DIP_MEASUREMENT_H

#include <map>
#include <vector>
#include <memory> // for unique_ptr

#include "diplib.h"
#include "chain_code.h"
#include "diplib/iterators.h"


/// \file
/// \brief Declares the `dip::Measurement` and `dip::MeasurementTool` classes, and the `dip::Feature` namespace.
/// \see measurement, infrastructure


namespace dip {


/// \defgroup measurement Measurement
/// \ingroup infrastructure
/// \brief The measurement infrastructure.


/// \brief Contains classes that implement the measurement features.
/// \ingroup measurement
namespace Feature {

/// \brief The types of measurement features
enum class Type {
      LINE_BASED, ///< The feature is derived from `dip::Feature::LineBased`
      IMAGE_BASED, ///< The feature is derived from `dip::Feature::ImageBased`
      CHAINCODE_BASED, ///< The feature is derived from `dip::Feature::ChainCodeBased`
      CONVEXHULL_BASED, ///< The feature is derived from `dip::Feature::ConvexHullBased`
      COMPOSITE ///< The feature is derived from `dip::Feature::Composite`
};

/// \brief %Information about a measurement feature
struct Information {
   String name;               ///< The name of the feature, used to identify it
   String description;        ///< A description of the feature, to be shown to the user
   bool needsGreyValue;       ///< Does the feature need a grey-value image?
   Information( String const& name, String const& description, bool needsGreyValue = false ) :
         name( name ), description( description ), needsGreyValue( needsGreyValue ) {}
   Information() : name( "" ), description( "" ), needsGreyValue( false ) {}
};

/// \brief %Information about the known measurement features
using InformationArray = DimensionArray< Information >;

/// \brief %Information about a measurement value, one of the components of a feature
struct ValueInformation {
   String name; ///< A short string that identifies the value
   Units units; ///< The units for the value
};

/// \brief %Information about the values of a measurement feature, or all values of all measurement features
/// in a `dip::Measurement` object.
using ValueInformationArray = std::vector< ValueInformation >;

} // namespace Feature


/// \brief Contains measurement results, as obtained through `dip::MeasurementTool::Measure`.
///
/// \ingroup measurement
///
/// A newly constructed `%Measurement` will accept calls to `AddFeature`, and
/// `AddObjectIDs`. Once the object is set up with all objects and features needed, a call
/// to `Forge` creates the data segment necessary to hold all those measurements. Once
/// forged, it is no longer possible to add features or objects.
///
/// A forged `%Measurement` can be read from in various ways, and a writeable pointer to the
/// data can be obtained.
///
/// The columns of the `%Measurement` table are the features. Each feature can have multiple
/// values, and so can have multiple consecutive sub-columns. The rows of the table are the objects.
///
/// Indexing with a feature name produces a reference to a column. Indexing with an object ID
/// (an integer) produces a reference to a row. Each of these references can be indexed to
/// produce a reference to a table cell. A cell contains the values produced by one feature for one
/// object. The cell group can again be indexed to obtain each of the values. These three types of
/// references are represented as iterators. Thus, it is also possible to iterate over all columns
/// (or all rows), iterate over each of the cell groups within a column (or within a row), and
/// iterate over the values within a cell group.
class Measurement {
   public:
      using ValueType = dfloat;           ///< The type of the measurement data
      using ValueIterator = ValueType*;   ///< A pointer to measurement data, which we can treat as an iterator

      /// \brief Structre containing information about the features stored in a `dip::Measurement` object
      struct FeatureInfo {
         String name;            ///< Name of the feature
         dip::uint startColumn;  ///< Column for first value of feature
         dip::uint numberValues; ///< Number of vales in feature
         FeatureInfo( String const& name, dip::uint startColumn, dip::uint numberValues )
               : name( name ), startColumn( startColumn ), numberValues( numberValues ) {}
      };

      /// \brief An iterator to visit all features (columns) in the `dip::Measurement` table. Can also be seen as a
      /// view over a specific feature.
      ///
      /// The iterator can be indexed with an object ID to access the table cell that contains the feature's
      /// values for that object. It is also possible to iterate over all objects. See `dip::Measurement` for
      /// examples of using this class.
      class IteratorFeature {
         public:
            friend class Measurement;
            /// \brief An iterator to visit all objects (rows) within a feature (column) of the `dip::Measurement` table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class Iterator {
               public:
                  friend class IteratorFeature;
                  /// \brief Index to acess a specific value
                  ValueType& operator[]( dip::uint index ) { return *( begin() + index ); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() {
                     return feature_.measurement_.data_.data() +
                           index_ * feature_.measurement_.Stride() +
                           feature_.Feature().startColumn;
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueIterator data() { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return feature_.Feature().numberValues; }
                  /// \brief Increment, to access the next object
                  Iterator& operator++() { ++index_; return *this; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return index_ >= feature_.NumberOfObjects(); }
                  /// \brief True if the iterator is valid and can be used
                  operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& Name() const { return feature_.Name(); }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return feature_.measurement_.objects_[ index_ ]; }
               private:
                  Iterator( IteratorFeature& feature, dip::uint index ) : feature_( feature ), index_( index ) {}
                  IteratorFeature& feature_;
                  dip::uint index_;
            };
            /// \brief Iterator to the first object for this feature
            Iterator FirstObject() { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given object for this feature
            Iterator operator[]( dip::uint objectID ) { return Iterator( *this, ObjectIndex( objectID )); }
            /// \brief Increment, to access the next feature
            IteratorFeature& operator++() { ++index_; return *this; }
            /// \brief True if done iterating (do not call other methods if this is true!)
            bool IsAtEnd() const { return index_ >= NumberOfFeatures(); }
            /// \brief True if the iterator is valid and can be used
            operator bool() const { return !IsAtEnd(); }
            /// \brief Name of the feature
            String const& Name() const { return Feature().name; }
            /// \brief Number of objects
            dip::uint NumberOfObjects() const { return measurement_.NumberOfObjects(); }
            /// \brief Returns a list of object IDs
            UnsignedArray const& Objects() const { return measurement_.Objects(); }
            /// \brief Finds the index for the given object ID
            dip::uint ObjectIndex( dip::uint objectID ) const { return measurement_.ObjectIndex( objectID ); }
         private:
            IteratorFeature( Measurement& measurement, dip::uint index ) : measurement_( measurement ), index_( index ) {}
            dip::uint NumberOfFeatures() const { return measurement_.NumberOfFeatures(); }
            FeatureInfo const& Feature() const { return measurement_.features_[ index_ ]; }
            Measurement& measurement_;
            dip::uint index_;
      };

      /// \brief An iterator to visit all objects (rows) in the `dip::Measurement` table. Can also be seen as a
      /// view over a specific object.
      ///
      /// The iterator can be indexed with an feature name to access the table cell that contains the object's
      /// values for that feature. It is also possible to iterate over all features. See `dip::Measurement` for
      /// examples of using this class.
      class IteratorObject {
         public:
            friend class Measurement;
            /// \brief An iterator to visit all features (columns) within an object (row) of the `dip::Measurement` table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class Iterator {
               public:
                  friend class IteratorObject;
                  /// \brief Index to acess a specific value
                  ValueType& operator[]( dip::uint index ) { return *( begin() + index ); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() {
                     return object_.measurement_.Data() +
                           object_.index_ * object_.measurement_.Stride() +
                           Feature().startColumn;
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueIterator data() { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return Feature().numberValues; }
                  /// \brief Increment, to access the next feature
                  Iterator& operator++() { ++index_; return *this; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return index_ >= object_.NumberOfFeatures(); }
                  /// \brief True if the iterator is valid and can be used
                  operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& Name() const { return Feature().name; }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return object_.ObjectID(); }
               private:
                  Iterator( IteratorObject& object, dip::uint index ) : object_( object ), index_( index ) {}
                  FeatureInfo const& Feature() const { return object_.measurement_.features_[ index_ ]; }
                  IteratorObject& object_;
                  dip::uint index_;
            };
            /// \brief Iterator to the first feature for this object
            Iterator FirstFeature() { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given feature for this object
            Iterator operator[]( String const& name ) { return Iterator( *this, FeatureIndex( name )); }
            /// \brief Increment, to access the next object
            IteratorObject& operator++() { ++index_; return *this; }
            /// \brief True if done iterating (do not call other methods if this is true!)
            bool IsAtEnd() const { return index_ >= NumberOfObjects(); }
            /// \brief True if the iterator is valid and can be used
            operator bool() const { return !IsAtEnd(); }
            /// \brief ID of object
            dip::uint ObjectID() const { return measurement_.objects_[ index_ ]; }
            /// \brief Number of features
            dip::uint NumberOfFeatures() const { return measurement_.NumberOfFeatures(); }
            /// \brief Returns an array of feature names
            std::vector< FeatureInfo > const& Features() const { return measurement_.Features(); }
            /// \brief Returns the index to the first columns for the feature
            dip::uint ValueIndex( String const& name ) const { return measurement_.ValueIndex( name ); }
         private:
            IteratorObject( Measurement& measurement, dip::uint index ) : measurement_( measurement ), index_( index ) {}
            dip::uint NumberOfObjects() const { return measurement_.NumberOfObjects(); }
            dip::uint FeatureIndex( String const& name ) const { return measurement_.FeatureIndex( name ); }
            Measurement& measurement_;
            dip::uint index_;
      };

      /// \brief Adds a feature to a non-forged `Measurement` object.
      void AddFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged." );
         DIP_THROW_IF( name.empty(), "No feature name given." );
         DIP_THROW_IF( FeatureExists( name ), String( "Feature already present: " ) + name );
         DIP_THROW_IF( values.empty(), "A feature needs at least one value." );
         AddFeature_( name, values );
      }

      /// \brief Adds a feature to a non-forged `Measurement` object if it is not already there.
      void EnsureFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged." );
         DIP_THROW_IF( name.empty(), "No feature name given." );
         if( FeatureExists( name )) {
            return;
         }
         DIP_THROW_IF( values.empty(), "A feature needs at least one value." );
         AddFeature_( name, values );
      }

      /// \brief Adds object IDs to a non-forged `Measurement` object.
      void AddObjectIDs( UnsignedArray const& objectIDs ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged." );
         for( auto const& objectID : objectIDs ) {
            DIP_THROW_IF( ObjectExists( objectID ), String( "Object already present: " ) + std::to_string( objectID ));
            dip::uint index = objects_.size();
            objects_.push_back( objectID );
            objectIndices_.emplace( objectID, index );
         }
      }

      /// \brief Forges the table, allocating space to hold measurement values.
      void Forge() {
         if( !IsForged() ) {
            dip::uint n = values_.size() * objects_.size();
            DIP_THROW_IF( n == 0, "Attempting to forge a zero-sized table." );
            data_.resize( n );
         }
      }

      /// \brief Creates an iterator (view) to the first object
      IteratorObject FirstObject() { return IteratorObject( *this, 0 ); }

      /// \brief Creats and iterator (view) to the given object
      IteratorObject operator[]( dip::uint objectID ) { return IteratorObject( *this, ObjectIndex( objectID )); }

      /// \brief Creats and iterator (view) to the first feature
      IteratorFeature FirstFeature() { return IteratorFeature( *this, 0 ); }

      /// \brief Creats and iterator (view) to the given feature
      IteratorFeature operator[]( String const& name ) { return IteratorFeature( *this, FeatureIndex( name )); }

      /// \brief A raw pointer to the data of the table. All values for one object are contiguous.
      ValueType* Data() {
         DIP_THROW_IF( !IsForged(), "Measurement object not forged." );
         return data_.data();
      }

      /// \brief A raw pointer to the data of the table. All values for one object are contiguous.
      ValueType const* Data() const {
         DIP_THROW_IF( !IsForged(), "Measurement object not forged." );
         return data_.data();
      }

      /// \brief The stride to use to access the next row of data in the table (next object).
      dip::sint Stride() const {
         return values_.size();
      }

      /// \brief True if the feature is available in `this`.
      bool FeatureExists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      /// \brief Finds the column index for the first value of the given feature.
      dip::uint FeatureIndex( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), String( "Feature not present: " ) + name );
         return it->second;
      }

      /// \brief Returns an array of feature names
      std::vector< FeatureInfo > const& Features() const {
         return features_;
      }

      /// \brief Returns the number of features
      dip::uint NumberOfFeatures() const {
         return features_.size();
      }

      /// \brief Finds the index into the `dip::Measurement::Values()` array for the first value of the given feature.
      dip::uint ValueIndex( String const& name ) const {
         return features_[ FeatureIndex( name ) ].startColumn;
      }

      /// \brief Returns an array with names and units for each of the values for the feature.
      /// (Note: data is copied to output array, not a trivial function).
      Feature::ValueInformationArray Values( String const& name ) const {
         auto feature = features_[ FeatureIndex( name ) ];
         Feature::ValueInformationArray values( feature.numberValues );
         for( dip::uint ii = 0; ii < feature.numberValues; ++ii ) {
            values[ ii ] = values_[ ii + feature.startColumn ];
         }
         return values;
      }

      /// \brief Returns an array with names and units for each of the values (for all features)
      Feature::ValueInformationArray const& Values() const {
         return values_;
      }

      /// \brief Returns the total number of feature values
      dip::uint NumberOfValues() const {
         return values_.size();
      }

      /// \brief Returns the number of values for the given feature
      dip::uint NumberOfValues( String const& name ) const {
         dip::uint index = FeatureIndex( name );
         return features_[ index ].numberValues;
      }

      /// \brief True if the object ID is available in `this`.
      bool ObjectExists( dip::uint objectID ) const {
         return objectIndices_.count( objectID ) != 0;
      }

      /// \brief Finds the row index for the given object ID.
      dip::uint ObjectIndex( dip::uint objectID ) const {
         auto it = objectIndices_.find( objectID );
         DIP_THROW_IF( it == objectIndices_.end(), String( "Object not present: " ) + std::to_string( objectID ));
         return it->second;
      }

      /// \brief Returns a list of object IDs
      UnsignedArray const& Objects() const {
          return objects_;
      }

      /// \brief Returns the number of objects
      dip::uint NumberOfObjects() const {
         return objects_.size();
      }

   private:

      bool IsForged() const { return !data_.empty(); }
      void AddFeature_( String const& name, Feature::ValueInformationArray const& values ) {
         dip::uint startIndex = values_.size();
         values_.resize( startIndex + values.size() );
         for( dip::uint ii = 0; ii < values.size(); ++ii ) {
            values_[ startIndex + ii ] = values[ ii ];
         }
         dip::uint index = features_.size();
         features_.emplace_back( name, startIndex, values.size() );
         featureIndices_.emplace( name, index );
      }
      UnsignedArray objects_;                         // the rows of the table (maps row indices to objectIDs)
      std::map< dip::uint, dip::uint > objectIndices_;// maps object IDs to row indices
      std::vector< FeatureInfo > features_;           // the columns of the table (maps column indices to feature names and contains other indo also)
      Feature::ValueInformationArray values_;         // the sub-columns of the table
      std::map< String, dip::uint > featureIndices_;  // maps feature names to column indices
      std::vector< ValueType > data_;
      // `data` has a row for each objectID, and a column for each feature value. The rows are stored contiguous.
      // `data[ features[ ii ].offset + jj * numberValues ]` gives the first value for feature `ii` for object with
      // index `jj`. `jj = objectIndices_[ id ]`. `ii = features_[ featureIndices_[ name ]].startColumn`.
};

//
// Overloaded operators
//

/// \brief You can output a `dip::Image` to `std::cout` or any other stream. Some
/// information about the image is printed.
std::ostream& operator<<( std::ostream& os, Measurement const& msr );


namespace Feature {

/// \brief The pure virtual base class for all measurement features.
class Base {
   public:
      Information const information; ///< Information on the feature
      Type const type; ///< The type of the measurement

      Base( Information const& information, Type const type ) : information( information ), type( type ) {};

      /// \brief All measurement features define an `Initialize` method that prepares the feature class
      /// to perform measurements on the image. It also gives information on the feature as applied to that image.
      ///
      /// This function should check image properties and throw an exception if the measurement
      /// cannot be made. The `dip::MeasurementTool` will not catch this exception, please provide a
      /// meaningful error message for the user. `label` will always be a scalar, unsigned integer image, and
      /// `grey` will always be of a real type. But `grey` can be a tensor image, so do check for that. For
      /// chain-code--based and convex-hull--based measurements, the images will always have exactly two
      /// dimensions; for other measurement types, the images will have at least one dimension, check the
      /// image dimensionality if there are other constraints. `grey` will always have the same dimensionality
      /// and sizes as `label` if the measurement requires a grey-value image; it will be a raw image otherwise.
      ///
      /// %Information returned includes the number of output values it will generate per object, what
      /// their name and units will be, and how many intermediate values it will need to store (for
      /// line-based functions only).
      ///
      /// Note that this function can store information about the images in private data members of the
      /// class, so that it is available when performing measurements. For example, it can store the
      /// pixel size to inform the measurement.
      ///
      /// Note that this function is not expected perform any major amount of work.
      virtual ValueInformationArray Initialize( Image const& label, Image const& grey ) = 0;

      /// \brief All measurement features define a `Cleanup` method that is called after finishing the measurement
      /// process for one image.
      virtual void Cleanup() = 0;

      virtual ~Base() {};
};

/// \brief A pointer to a measurement feature of any type
using Pointer = std::unique_ptr< Base >;

/// \brief The pure virtual base class for all line-based measurement features.
class LineBased : public Base {
   public:
      LineBased( Information const& information ) : Base( information, Type::LINE_BASED ) {};

      /// \brief Called once for each image line, to accumulate information about each object.
      /// This function is not called in parallel, and hence does not need to be re-entrant.
      virtual void Measure(
            LineIterator< uint32 > label, ///< Pointer to the line in the labelled image (always scalar)
            LineIterator< dfloat > grey, ///< Pointer to the line in the grey-value image (if given, invalid otherwise)
            UnsignedArray coordinates, ///< Coordinates of the first pixel on the line (by copy, so it can be modified)
            dip::uint dimension ///< Along which dimension the line runs
      ) = 0;

      /// \brief Called once for each object, to finalize the measurement
      virtual void Finish( dip::uint objectID, Measurement::ValueIterator data ) = 0;
};

/// \brief The pure virtual base class for all image-based measurement features.
class ImageBased : public Base {
   public:
      ImageBased( Information const& information ) : Base( information, Type::IMAGE_BASED ) {};

      /// \brief Called once to compute measurements for all objects
      virtual void Measure( Image const& label, Image const& grey, Measurement::IteratorFeature& data ) = 0;
};

/// \brief The pure virtual base class for all chain-code--based measurement features.
class ChainCodeBased : public Base {
   public:
      ChainCodeBased( Information const& information ) : Base( information, Type::CHAINCODE_BASED ) {};

      /// \brief Called once for each object
      virtual void Measure( ChainCode const& chainCode, Measurement::ValueIterator data ) = 0;
};

/// \brief The pure virtual base class for all convex-hull--based measurement features.
class ConvexHullBased : public Base {
   public:
      ConvexHullBased( Information const& information ) : Base( information, Type::CONVEXHULL_BASED ) {};

      /// \brief Called once for each object
      virtual void Measure( ConvexHull const& convexHull, Measurement::ValueIterator data ) = 0;
};

/// \brief The pure virtual base class for all composite measurement features.
class Composite : public Base {
   public:
      Composite( Information const& information ) : Base( information, Type::COMPOSITE ) {};

      /// \brief Lists the features that the measurement depends on. These features will be computed and made
      /// available to the `Measure` method. This function is always called after `dip::Feature::Base::Initialize`.
      virtual StringArray Dependencies() = 0;

      /// \brief Called once for each object, the input `dependencies` object contains the measurements
      /// for the object from all the features in the `dip::Composite::Dependencies` list.
      virtual void Measure( Measurement::IteratorObject& dependencies, Measurement::ValueIterator data ) = 0;
};

} // namespace Feature


/// \brief Performs measurements, as defined by classes in `dip::Feature`, on images.
///
/// \ingroup measurement
///
/// The MeasurementTool class knows about defined measurement features, and can apply them to an
/// image through its `dip::MeasurementTool::Measure` method.
///
///     dip::MeasurementTool tool;
///     dip::Image img = ...
///     dip::Image label = Label( Threshold( img ), 2 );
///     dip::Measurement msr = tool.Measure( label, img, { "Size", "Perimeter" }, {}, 2 );
///     std::cout << "Size of object with label 1 is " << msr[ "Size" ][ 1 ][ 0 ] << std::endl;
///
// TODO: Document default measurement features here.
class MeasurementTool {
   public:

      /// \brief Constructor.
      MeasurementTool();

      /// \brief Registers a feature with this `MeasurementTool`. The feature object becomes property of the tool.
      ///
      /// Create an instance of the feature class on the heap using `new`. The feature class must be
      /// derived from one of the five classes dervied from `dip::Feature::Base` (thus not directly from `Base`).
      /// Note that the pointer returned by `new` must be explicitly converted to a `dip::Feature::Pointer`:
      ///
      ///     class MyFeature : public dip::Feature::ChainCodeBased {
      ///        // define constructor and override virtual functions
      ///     }
      ///     MeasurementTool measurementTool;
      ///     measurementTool.Register( dip::Feature::Pointer( new MyFeature ));
      void Register(
            Feature::Pointer feature
      ) {
         String const& name = feature->information.name;
         if( !Exists( name )) {
            dip::uint index = features_.size();
            features_.emplace_back( std::move( feature ));
            featureIndices_.emplace( name, index );
         } else {
            feature = nullptr; // deallocates the feature we received, we don't need it, we already have one with that name
         }
      }

      /// \brief Measures one or more features on one or more objects in the labelled image.
      ///
      /// `label` is a labelled image (any unsigned integer type, and scalar), and `grey` is either a raw
      /// image (not forged, without pixel data), or an real-valued image with the same dimensionaliry and
      /// sizes as `label`. If any selected features require a grey-value image, then it must be provided.
      ///
      /// `features` is an array with feature names. See the `dip::MeasurementTool::Features` method for
      /// information on how to obtain those names. Some features are composite features, they compute
      /// values based on other features. Thus, it is possible that the output `dip::Measurement` object
      /// contains features not directly requested, but needed to compute another feature.
      ///
      /// `objectIDs` is an array with the IDs of objects to measure, If any of the IDs is not a label
      /// in the `label` image, the resulting measures will be zero or otherwise marked as invalid. If
      /// an empty array is given, all objects in the labelled image are measured.
      ///
      /// `connectivity` should match the value used when creating the labelled image `label`.
      ///
      /// The output `dip::Measurement` structure contains measurements that take the pixel size of
      /// the `label` image into account. Those of `grey` are ignored. Some measurements require
      /// isotropic pixel sizes, if `label` is not isotropic, the pixel size is ignored and these
      /// measures will return values in pixels instead.
      Measurement Measure(
            Image const& label,
            Image const& grey,
            StringArray features, // we take a copy of this array
            UnsignedArray const& objectIDs,
            dip::uint connectivity = 2
      ) const;

      /// \brief Returns a table with known feature names and descriptions, which can directly be shown to the user.
      /// (Note: data is copied to output array, not a trivial function).
      Feature::InformationArray Features() const {
         Feature::InformationArray out;
         for( auto const& feature : features_ ) {
            out.push_back( feature->information );
         }
         return out;
      }

   private:

      std::vector< Feature::Pointer > features_;
      std::map< String, dip::uint > featureIndices_;

      bool Exists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      dip::uint Index( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), String( "Feature name not known: " ) + name );
         return it->second;
      }
};


/// \brief Paints each object with the selected measurement feature values.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. It is assumed that
/// that object was obtained through measurement of the input `label` image.
///
/// If the selected feature has more than one value, then `out` will be a vector image with as many tensor elements
/// as values are in the feature.
// TODO: We could try to allow the user to "tweak" the IteratorFeature object so that it points to a custom
//       subset of value columns (i.e. a single value of a feature, or any consecutive set of values).
void ObjectToMeasurement(
      Image const& label,
      Image& out,
      Measurement::IteratorFeature const& featureValues
);

inline Image ObjectToMeasurement(
      Image const& label,
      Measurement::IteratorFeature const& featureValues
) {
   Image out;
   ObjectToMeasurement( label, out, featureValues );
   return out;
}


} // namespace dip

#endif // DIP_MEASUREMENT_H
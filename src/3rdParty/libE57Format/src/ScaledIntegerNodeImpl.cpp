/*
 * Original work Copyright 2009 - 2010 Kevin Ackley (kackley@gwi.net)
 * Modified work Copyright 2018 - 2020 Andy Maloney <asmaloney@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <cmath>

#include "CheckedFile.h"
#include "ScaledIntegerNodeImpl.h"

namespace e57
{
   ScaledIntegerNodeImpl::ScaledIntegerNodeImpl( ImageFileImplWeakPtr destImageFile, int64_t rawValue, int64_t minimum,
                                                 int64_t maximum, double scale, double offset ) :
      NodeImpl( destImageFile ), value_( rawValue ), minimum_( minimum ), maximum_( maximum ), scale_( scale ),
      offset_( offset )
   {
      // don't checkImageFileOpen, NodeImpl() will do it

      /// Enforce the given bounds on raw value
      if ( rawValue < minimum || maximum < rawValue )
      {
         throw E57_EXCEPTION2( E57_ERROR_VALUE_OUT_OF_BOUNDS,
                               "this->pathName=" + this->pathName() + " rawValue=" + toString( rawValue ) +
                                  " minimum=" + toString( minimum ) + " maximum=" + toString( maximum ) );
      }
   }

   ScaledIntegerNodeImpl::ScaledIntegerNodeImpl( ImageFileImplWeakPtr destImageFile, double scaledValue,
                                                 double scaledMinimum, double scaledMaximum, double scale,
                                                 double offset ) :
      NodeImpl( destImageFile ), value_( static_cast<int64_t>( std::floor( ( scaledValue - offset ) / scale + .5 ) ) ),
      minimum_( static_cast<int64_t>( std::floor( ( scaledMinimum - offset ) / scale + .5 ) ) ),
      maximum_( static_cast<int64_t>( std::floor( ( scaledMaximum - offset ) / scale + .5 ) ) ), scale_( scale ),
      offset_( offset )
   {
      // don't checkImageFileOpen, NodeImpl() will do it

      /// Enforce the given bounds on raw value
      if ( scaledValue < scaledMinimum || scaledMaximum < scaledValue )
      {
         throw E57_EXCEPTION2( E57_ERROR_VALUE_OUT_OF_BOUNDS, "this->pathName=" + this->pathName() +
                                                                 " scaledValue=" + toString( scaledValue ) +
                                                                 " scaledMinimum=" + toString( scaledMinimum ) +
                                                                 " scaledMaximum=" + toString( scaledMaximum ) );
      }
   }

   bool ScaledIntegerNodeImpl::isTypeEquivalent( NodeImplSharedPtr ni )
   {
      // don't checkImageFileOpen

      /// Same node type?
      if ( ni->type() != E57_SCALED_INTEGER )
      {
         return ( false );
      }

      /// Downcast to shared_ptr<ScaledIntegerNodeImpl>
      std::shared_ptr<ScaledIntegerNodeImpl> ii( std::static_pointer_cast<ScaledIntegerNodeImpl>( ni ) );

      /// minimum must match
      if ( minimum_ != ii->minimum_ )
      {
         return ( false );
      }

      /// maximum must match
      if ( maximum_ != ii->maximum_ )
      {
         return ( false );
      }

      /// scale must match
      if ( scale_ != ii->scale_ )
      {
         return ( false );
      }

      /// offset must match
      if ( offset_ != ii->offset_ )
      {
         return ( false );
      }

      /// ignore value_, doesn't have to match

      /// Types match
      return ( true );
   }

   bool ScaledIntegerNodeImpl::isDefined( const ustring &pathName )
   {
      // don't checkImageFileOpen

      /// We have no sub-structure, so if path not empty return false
      return pathName.empty();
   }

   int64_t ScaledIntegerNodeImpl::rawValue()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( value_ );
   }

   double ScaledIntegerNodeImpl::scaledValue()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( value_ * scale_ + offset_ );
   }

   int64_t ScaledIntegerNodeImpl::minimum()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( minimum_ );
   }
   double ScaledIntegerNodeImpl::scaledMinimum()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( minimum_ * scale_ + offset_ );
   }

   int64_t ScaledIntegerNodeImpl::maximum()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( maximum_ );
   }
   double ScaledIntegerNodeImpl::scaledMaximum()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( maximum_ * scale_ + offset_ );
   }

   double ScaledIntegerNodeImpl::scale()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( scale_ );
   }

   double ScaledIntegerNodeImpl::offset()
   {
      checkImageFileOpen( __FILE__, __LINE__, static_cast<const char *>( __FUNCTION__ ) );
      return ( offset_ );
   }

   void ScaledIntegerNodeImpl::checkLeavesInSet( const StringSet &pathNames, NodeImplSharedPtr origin )
   {
      // don't checkImageFileOpen

      /// We are a leaf node, so verify that we are listed in set.
      if ( pathNames.find( relativePathName( origin ) ) == pathNames.end() )
      {
         throw E57_EXCEPTION2( E57_ERROR_NO_BUFFER_FOR_ELEMENT, "this->pathName=" + this->pathName() );
      }
   }

   void ScaledIntegerNodeImpl::writeXml( ImageFileImplSharedPtr /*imf*/, CheckedFile &cf, int indent,
                                         const char *forcedFieldName )
   {
      // don't checkImageFileOpen

      ustring fieldName;
      if ( forcedFieldName )
      {
         fieldName = forcedFieldName;
      }
      else
      {
         fieldName = elementName_;
      }

      cf << space( indent ) << "<" << fieldName << " type=\"ScaledInteger\"";

      /// Don't need to write if are default values
      if ( minimum_ != E57_INT64_MIN )
      {
         cf << " minimum=\"" << minimum_ << "\"";
      }
      if ( maximum_ != E57_INT64_MAX )
      {
         cf << " maximum=\"" << maximum_ << "\"";
      }
      if ( scale_ != 1.0 )
      {
         cf << " scale=\"" << scale_ << "\"";
      }
      if ( offset_ != 0.0 )
      {
         cf << " offset=\"" << offset_ << "\"";
      }

      /// Write value as child text, unless it is the default value
      if ( value_ != 0 )
      {
         cf << ">" << value_ << "</" << fieldName << ">\n";
      }
      else
      {
         cf << "/>\n";
      }
   }

#ifdef E57_DEBUG
   void ScaledIntegerNodeImpl::dump( int indent, std::ostream &os ) const
   {
      // don't checkImageFileOpen
      os << space( indent ) << "type:        ScaledInteger" << " (" << type() << ")" << std::endl;
      NodeImpl::dump( indent, os );
      os << space( indent ) << "rawValue:    " << value_ << std::endl;
      os << space( indent ) << "minimum:     " << minimum_ << std::endl;
      os << space( indent ) << "maximum:     " << maximum_ << std::endl;
      os << space( indent ) << "scale:       " << scale_ << std::endl;
      os << space( indent ) << "offset:      " << offset_ << std::endl;
   }
#endif
}

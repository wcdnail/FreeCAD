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

#include "ImageFileImpl.h"
#include "SourceDestBufferImpl.h"

using namespace e57;

SourceDestBufferImpl::SourceDestBufferImpl( ImageFileImplWeakPtr destImageFile, const ustring &pathName,
                                            const size_t capacity, bool doConversion, bool doScaling ) :
   destImageFile_( destImageFile ), pathName_( pathName ), capacity_( capacity ), doConversion_( doConversion ),
   doScaling_( doScaling )
{
}

template <typename T> void SourceDestBufferImpl::setTypeInfo( T *base, size_t stride )
{
   static_assert( std::is_integral<T>::value || std::is_floating_point<T>::value,
                  "Integral or floating point required." );

   base_ = reinterpret_cast<char *>( base );
   stride_ = stride;

   // this is a little ugly, but it saves us having to pass around the memory
   // representation
   if ( std::is_same<T, int8_t>::value )
   {
      memoryRepresentation_ = E57_INT8;
   }
   else if ( std::is_same<T, uint8_t>::value )
   {
      memoryRepresentation_ = E57_UINT8;
   }
   else if ( std::is_same<T, int16_t>::value )
   {
      memoryRepresentation_ = E57_INT16;
   }
   else if ( std::is_same<T, uint16_t>::value )
   {
      memoryRepresentation_ = E57_UINT16;
   }
   else if ( std::is_same<T, int32_t>::value )
   {
      memoryRepresentation_ = E57_INT32;
   }
   else if ( std::is_same<T, uint32_t>::value )
   {
      memoryRepresentation_ = E57_UINT32;
   }
   else if ( std::is_same<T, int64_t>::value )
   {
      memoryRepresentation_ = E57_INT64;
   }
   else if ( std::is_same<T, bool>::value )
   {
      memoryRepresentation_ = E57_BOOL;
   }
   else if ( std::is_same<T, float>::value )
   {
      memoryRepresentation_ = E57_REAL32;
   }
   else if ( std::is_same<T, double>::value )
   {
      memoryRepresentation_ = E57_REAL64;
   }

   checkState_();
}

template void SourceDestBufferImpl::setTypeInfo<int8_t>( int8_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<uint8_t>( uint8_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<int16_t>( int16_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<uint16_t>( uint16_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<int32_t>( int32_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<uint32_t>( uint32_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<int64_t>( int64_t *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<bool>( bool *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<float>( float *base, size_t stride );
template void SourceDestBufferImpl::setTypeInfo<double>( double *base, size_t stride );

SourceDestBufferImpl::SourceDestBufferImpl( ImageFileImplWeakPtr destImageFile, const ustring &pathName,
                                            std::vector<ustring> *b ) :
   destImageFile_( destImageFile ), pathName_( pathName ), memoryRepresentation_( E57_USTRING ), ustrings_( b )
{
   /// don't checkImageFileOpen, checkState_ will do it

   /// Set capacity_ after testing that b is OK
   if ( !b )
   {
      throw E57_EXCEPTION2( E57_ERROR_BAD_BUFFER, "sdbuf.pathName=" + pathName );
   }

   capacity_ = b->size();

   checkState_();

   /// Note that capacity_ is set to the size() of the vector<>, not its
   /// capacity(). The size() of *ustrings_ will not be changed as strings are
   /// stored in it.
}

template <typename T> void SourceDestBufferImpl::_setNextReal( T inValue )
{
   static_assert( std::is_same<T, double>::value || std::is_same<T, float>::value,
                  "_setNextReal() requires float or double type" );

   /// don't checkImageFileOpen

   /// Verify have room
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Calc start of memory location, index into buffer using stride_ (the
   /// distance between elements).
   char *p = &base_[nextIndex_ * stride_];

   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? fault if get special value: NaN, NegInf...  (all other ints below
         // too)
         if ( inValue < (T)E57_INT8_MIN || (T)E57_INT8_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<int8_t *>( p ) = static_cast<int8_t>( inValue );
         break;
      case E57_UINT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < (T)E57_UINT8_MIN || (T)E57_UINT8_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<uint8_t *>( p ) = static_cast<uint8_t>( inValue );
         break;
      case E57_INT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < E57_INT16_MIN || E57_INT16_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<int16_t *>( p ) = static_cast<int16_t>( inValue );
         break;
      case E57_UINT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < E57_UINT16_MIN || E57_UINT16_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<uint16_t *>( p ) = static_cast<uint16_t>( inValue );
         break;
      case E57_INT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < (T)E57_INT32_MIN || (T)E57_INT32_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<int32_t *>( p ) = static_cast<int32_t>( inValue );
         break;
      case E57_UINT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < (T)E57_UINT32_MIN || (T)E57_UINT32_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<uint32_t *>( p ) = static_cast<uint32_t>( inValue );
         break;
      case E57_INT64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         if ( inValue < (T)E57_INT64_MIN || (T)E57_INT64_MAX < inValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( inValue ) );
         }
         *reinterpret_cast<int64_t *>( p ) = static_cast<int64_t>( inValue );
         break;
      case E57_BOOL:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         *reinterpret_cast<bool *>( p ) = ( inValue ? false : true );
         break;
      case E57_REAL32:
         if ( std::is_same<T, double>::value )
         {
            /// Does this count as conversion?  It loses information.
            /// Check for really large exponents that can't fit in a single
            /// precision
            if ( inValue < (T)E57_DOUBLE_MIN || (T)E57_DOUBLE_MAX < inValue )
            {
               throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                     "pathName=" + pathName_ + " value=" + toString( inValue ) );
            }
            *reinterpret_cast<float *>( p ) = static_cast<float>( inValue );
         }
         else
         {
#ifdef _MSC_VER
            // MSVC is not smart enough to realize 'inValue' cannot be a double here, so disable warning
#pragma warning( disable : 4244 )
            *reinterpret_cast<float *>( p ) = inValue;
#pragma warning( default : 4244 )
#else
            *reinterpret_cast<float *>( p ) = inValue;
#endif
         }
         break;
      case E57_REAL64:
         //??? does this count as a conversion?
         *reinterpret_cast<double *>( p ) = static_cast<double>( inValue );
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
   }

   nextIndex_++;
}

void SourceDestBufferImpl::checkState_() const
{
   /// Implement checkImageFileOpen functionality for SourceDestBufferImpl ctors
   /// Throw an exception if destImageFile (destImageFile_) isn't open
   ImageFileImplSharedPtr destImageFile( destImageFile_ );
   if ( !destImageFile->isOpen() )
   {
      throw E57_EXCEPTION2( E57_ERROR_IMAGEFILE_NOT_OPEN, "fileName=" + destImageFile->fileName() );
   }

   /// Check pathName is well formed (can't verify path is defined until
   /// associate sdbuffer with CompressedVector later)
   ImageFileImplSharedPtr imf( destImageFile_ );
   imf->pathNameCheckWellFormed( pathName_ );

   if ( memoryRepresentation_ != E57_USTRING )
   {
      if ( !base_ )
      {
         throw E57_EXCEPTION2( E57_ERROR_BAD_BUFFER, "pathName=" + pathName_ );
      }
      if ( stride_ == 0 )
      {
         throw E57_EXCEPTION2( E57_ERROR_BAD_BUFFER, "pathName=" + pathName_ );
      }
      //??? check base alignment, depending on CPU type
      //??? check if stride too small, positive or negative
   }
   else
   {
      if ( !ustrings_ )
      {
         throw E57_EXCEPTION2( E57_ERROR_BAD_BUFFER, "pathName=" + pathName_ );
      }
   }
}

int64_t SourceDestBufferImpl::getNextInt64()
{
   /// don't checkImageFileOpen

   /// Verify index is within bounds
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Fetch value from source buffer.
   /// Convert from non-integer formats if requested.
   char *p = &base_[nextIndex_ * stride_];
   int64_t value;
   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         value = static_cast<int64_t>( *reinterpret_cast<int8_t *>( p ) );
         break;
      case E57_UINT8:
         value = static_cast<int64_t>( *reinterpret_cast<uint8_t *>( p ) );
         break;
      case E57_INT16:
         value = static_cast<int64_t>( *reinterpret_cast<int16_t *>( p ) );
         break;
      case E57_UINT16:
         value = static_cast<int64_t>( *reinterpret_cast<uint16_t *>( p ) );
         break;
      case E57_INT32:
         value = static_cast<int64_t>( *reinterpret_cast<int32_t *>( p ) );
         break;
      case E57_UINT32:
         value = static_cast<int64_t>( *reinterpret_cast<uint32_t *>( p ) );
         break;
      case E57_INT64:
         value = *reinterpret_cast<int64_t *>( p );
         break;
      case E57_BOOL:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         /// Convert bool to 0/1, all non-zero values map to 1.0
         value = ( *reinterpret_cast<bool *>( p ) ) ? 1 : 0;
         break;
      case E57_REAL32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? fault if get special value: NaN, NegInf...
         value = static_cast<int64_t>( *reinterpret_cast<float *>( p ) );
         break;
      case E57_REAL64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? fault if get special value: NaN, NegInf...
         value = static_cast<int64_t>( *reinterpret_cast<double *>( p ) );
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
      default:
         throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }
   nextIndex_++;
   return ( value );
}

int64_t SourceDestBufferImpl::getNextInt64( double scale, double offset )
{
   /// don't checkImageFileOpen

   /// Reverse scale (undo scaling) of a user's number to get raw value to put
   /// in file.

   /// Incorporating the scale is optional (requested by user when constructing
   /// the sdbuf). If the user did not request scaling, then we get raw values
   /// from user's buffer.
   if ( !doScaling_ )
   {
      /// Just return raw value.
      return ( getNextInt64() );
   }

   /// Double check non-zero scale.  Going to divide by it below.
   if ( scale == 0 )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Verify index is within bounds
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Fetch value from source buffer.
   /// Convert from non-integer formats if requested
   char *p = &base_[nextIndex_ * stride_];
   double doubleRawValue;
   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<int8_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_UINT8:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<uint8_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_INT16:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<int16_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_UINT16:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<uint16_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_INT32:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<int32_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_UINT32:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<uint32_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_INT64:
         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<int64_t *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_BOOL:
         if ( *reinterpret_cast<bool *>( p ) )
         {
            doubleRawValue = floor( ( 1 - offset ) / scale + 0.5 );
         }
         else
         {
            doubleRawValue = floor( ( 0 - offset ) / scale + 0.5 );
         }
         break;
      case E57_REAL32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? fault if get special value: NaN, NegInf...

         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<float *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_REAL64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? fault if get special value: NaN, NegInf...

         /// Calc (x-offset)/scale rounded to nearest integer, but keep in
         /// floating point until sure is in bounds
         doubleRawValue = floor( ( *reinterpret_cast<double *>( p ) - offset ) / scale + 0.5 );
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
      default:
         throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }
   /// Make sure that value is representable in an int64_t
   if ( doubleRawValue < (double)E57_INT64_MIN || (double)E57_INT64_MAX < doubleRawValue )
   {
      throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                            "pathName=" + pathName_ + " value=" + toString( doubleRawValue ) );
   }

   auto rawValue = static_cast<int64_t>( doubleRawValue );

   nextIndex_++;
   return ( rawValue );
}

float SourceDestBufferImpl::getNextFloat()
{
   /// don't checkImageFileOpen

   /// Verify index is within bounds
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Fetch value from source buffer.
   /// Convert from other formats to floating point if requested
   char *p = &base_[nextIndex_ * stride_];
   float value;
   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<int8_t *>( p ) );
         break;
      case E57_UINT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<uint8_t *>( p ) );
         break;
      case E57_INT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<int16_t *>( p ) );
         break;
      case E57_UINT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<uint16_t *>( p ) );
         break;
      case E57_INT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<int32_t *>( p ) );
         break;
      case E57_UINT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<uint32_t *>( p ) );
         break;
      case E57_INT64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<float>( *reinterpret_cast<int64_t *>( p ) );
         break;
      case E57_BOOL:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }

         /// Convert bool to 0/1, all non-zero values map to 1.0
         value = ( *reinterpret_cast<bool *>( p ) ) ? 1.0F : 0.0F;
         break;
      case E57_REAL32:
         value = *reinterpret_cast<float *>( p );
         break;
      case E57_REAL64:
      {
         /// Check that exponent of user's value is not too large for single
         /// precision number in file.
         double d = *reinterpret_cast<double *>( p );

         ///??? silently limit here?
         if ( d < E57_DOUBLE_MIN || E57_DOUBLE_MAX < d )
         {
            throw E57_EXCEPTION2( E57_ERROR_REAL64_TOO_LARGE, "pathName=" + pathName_ + " value=" + toString( d ) );
         }
         value = static_cast<float>( d );
         break;
      }
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
      default:
         throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }
   nextIndex_++;
   return ( value );
}

double SourceDestBufferImpl::getNextDouble()
{
   /// don't checkImageFileOpen

   /// Verify index is within bounds
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Fetch value from source buffer.
   /// Convert from other formats to floating point if requested
   char *p = &base_[nextIndex_ * stride_];
   double value;
   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<int8_t *>( p ) );
         break;
      case E57_UINT8:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<uint8_t *>( p ) );
         break;
      case E57_INT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<int16_t *>( p ) );
         break;
      case E57_UINT16:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<uint16_t *>( p ) );
         break;
      case E57_INT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<int32_t *>( p ) );
         break;
      case E57_UINT32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<uint32_t *>( p ) );
         break;
      case E57_INT64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         value = static_cast<double>( *reinterpret_cast<int64_t *>( p ) );
         break;
      case E57_BOOL:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         /// Convert bool to 0/1, all non-zero values map to 1.0
         value = ( *reinterpret_cast<bool *>( p ) ) ? 1.0 : 0.0;
         break;
      case E57_REAL32:
         value = static_cast<double>( *reinterpret_cast<float *>( p ) );
         break;
      case E57_REAL64:
         value = *reinterpret_cast<double *>( p );
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
      default:
         throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }
   nextIndex_++;
   return ( value );
}

ustring SourceDestBufferImpl::getNextString()
{
   /// don't checkImageFileOpen

   /// Check have correct type buffer
   if ( memoryRepresentation_ != E57_USTRING )
   {
      throw E57_EXCEPTION2( E57_ERROR_EXPECTING_USTRING, "pathName=" + pathName_ );
   }

   /// Verify index is within bounds
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Get ustring from vector
   return ( ( *ustrings_ )[nextIndex_++] );
}

void SourceDestBufferImpl::setNextInt64( int64_t value )
{
   /// don't checkImageFileOpen

   /// Verify have room
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Calc start of memory location, index into buffer using stride_ (the
   /// distance between elements).
   char *p = &base_[nextIndex_ * stride_];

   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         if ( value < E57_INT8_MIN || E57_INT8_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<int8_t *>( p ) = static_cast<int8_t>( value );
         break;
      case E57_UINT8:
         if ( value < E57_UINT8_MIN || E57_UINT8_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<uint8_t *>( p ) = static_cast<uint8_t>( value );
         break;
      case E57_INT16:
         if ( value < E57_INT16_MIN || E57_INT16_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<int16_t *>( p ) = static_cast<int16_t>( value );
         break;
      case E57_UINT16:
         if ( value < E57_UINT16_MIN || E57_UINT16_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<uint16_t *>( p ) = static_cast<uint16_t>( value );
         break;
      case E57_INT32:
         if ( value < E57_INT32_MIN || E57_INT32_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<int32_t *>( p ) = static_cast<int32_t>( value );
         break;
      case E57_UINT32:
         if ( value < E57_UINT32_MIN || E57_UINT32_MAX < value )
         {
            throw E57_EXCEPTION2( E57_ERROR_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " value=" + toString( value ) );
         }
         *reinterpret_cast<uint32_t *>( p ) = static_cast<uint32_t>( value );
         break;
      case E57_INT64:
         *reinterpret_cast<int64_t *>( p ) = static_cast<int64_t>( value );
         break;
      case E57_BOOL:
         *reinterpret_cast<bool *>( p ) = ( value ? false : true );
         break;
      case E57_REAL32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         //??? very large integers may lose some lowest bits here. error?
         *reinterpret_cast<float *>( p ) = static_cast<float>( value );
         break;
      case E57_REAL64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         *reinterpret_cast<double *>( p ) = static_cast<double>( value );
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
   }

   nextIndex_++;
}

void SourceDestBufferImpl::setNextInt64( int64_t value, double scale, double offset )
{
   /// don't checkImageFileOpen

   /// Apply a scale and offset to numbers from file before putting in user's
   /// buffer.

   /// Incorporating the scale is optional (requested by user when constructing
   /// the sdbuf). If the user did not request scaling, then we send raw values
   /// to user's buffer.
   if ( !doScaling_ )
   {
      /// Use raw value routine, then bail out.
      setNextInt64( value );
      return;
   }

   /// Verify have room
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Calc start of memory location, index into buffer using stride_ (the
   /// distance between elements).
   char *p = &base_[nextIndex_ * stride_];

   /// Calc x*scale+offset
   double scaledValue;
   if ( memoryRepresentation_ == E57_REAL32 || memoryRepresentation_ == E57_REAL64 )
   {
      /// Value will be stored in some floating point rep in user's buffer, so
      /// keep full resolution here.
      scaledValue = value * scale + offset;
   }
   else
   {
      /// Value will represented as some integer in user's buffer, so round to
      /// nearest integer here. But keep in floating point rep until we know
      /// that the value is representable in the user's buffer.
      scaledValue = floor( value * scale + offset + 0.5 );
   }

   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         if ( scaledValue < E57_INT8_MIN || E57_INT8_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<int8_t *>( p ) = static_cast<int8_t>( scaledValue );
         break;
      case E57_UINT8:
         if ( scaledValue < E57_UINT8_MIN || E57_UINT8_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<uint8_t *>( p ) = static_cast<uint8_t>( scaledValue );
         break;
      case E57_INT16:
         if ( scaledValue < E57_INT16_MIN || E57_INT16_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<int16_t *>( p ) = static_cast<int16_t>( scaledValue );
         break;
      case E57_UINT16:
         if ( scaledValue < E57_UINT16_MIN || E57_UINT16_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<uint16_t *>( p ) = static_cast<uint16_t>( scaledValue );
         break;
      case E57_INT32:
         if ( scaledValue < E57_INT32_MIN || E57_INT32_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<int32_t *>( p ) = static_cast<int32_t>( scaledValue );
         break;
      case E57_UINT32:
         if ( scaledValue < E57_UINT32_MIN || E57_UINT32_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<uint32_t *>( p ) = static_cast<uint32_t>( scaledValue );
         break;
      case E57_INT64:
         *reinterpret_cast<int64_t *>( p ) = static_cast<int64_t>( scaledValue );
         break;
      case E57_BOOL:
         *reinterpret_cast<bool *>( p ) = ( scaledValue ? false : true );
         break;
      case E57_REAL32:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         /// Check that exponent of result is not too big for single precision
         /// float
         if ( scaledValue < E57_DOUBLE_MIN || E57_DOUBLE_MAX < scaledValue )
         {
            throw E57_EXCEPTION2( E57_ERROR_SCALED_VALUE_NOT_REPRESENTABLE,
                                  "pathName=" + pathName_ + " scaledValue=" + toString( scaledValue ) );
         }
         *reinterpret_cast<float *>( p ) = static_cast<float>( scaledValue );
         break;
      case E57_REAL64:
         if ( !doConversion_ )
         {
            throw E57_EXCEPTION2( E57_ERROR_CONVERSION_REQUIRED, "pathName=" + pathName_ );
         }
         *reinterpret_cast<double *>( p ) = scaledValue;
         break;
      case E57_USTRING:
         throw E57_EXCEPTION2( E57_ERROR_EXPECTING_NUMERIC, "pathName=" + pathName_ );
   }

   nextIndex_++;
}

void SourceDestBufferImpl::setNextFloat( float value )
{
   _setNextReal( value );
}

void SourceDestBufferImpl::setNextDouble( double value )
{
   _setNextReal( value );
}

void SourceDestBufferImpl::setNextString( const ustring &value )
{
   /// don't checkImageFileOpen

   if ( memoryRepresentation_ != E57_USTRING )
   {
      throw E57_EXCEPTION2( E57_ERROR_EXPECTING_USTRING, "pathName=" + pathName_ );
   }

   /// Verify have room.
   if ( nextIndex_ >= capacity_ )
   {
      throw E57_EXCEPTION2( E57_ERROR_INTERNAL, "pathName=" + pathName_ );
   }

   /// Assign to already initialized element in vector
   ( *ustrings_ )[nextIndex_] = value;
   nextIndex_++;
}

void SourceDestBufferImpl::checkCompatible( const std::shared_ptr<SourceDestBufferImpl> &newBuf ) const
{
   if ( pathName_ != newBuf->pathName() )
   {
      throw E57_EXCEPTION2( E57_ERROR_BUFFERS_NOT_COMPATIBLE,
                            "pathName=" + pathName_ + " newPathName=" + newBuf->pathName() );
   }
   if ( memoryRepresentation_ != newBuf->memoryRepresentation() )
   {
      throw E57_EXCEPTION2( E57_ERROR_BUFFERS_NOT_COMPATIBLE,
                            "memoryRepresentation=" + toString( memoryRepresentation_ ) +
                               " newMemoryType=" + toString( newBuf->memoryRepresentation() ) );
   }
   if ( capacity_ != newBuf->capacity() )
   {
      throw E57_EXCEPTION2( E57_ERROR_BUFFERS_NOT_COMPATIBLE,
                            "capacity=" + toString( capacity_ ) + " newCapacity=" + toString( newBuf->capacity() ) );
   }
   if ( doConversion_ != newBuf->doConversion() )
   {
      throw E57_EXCEPTION2( E57_ERROR_BUFFERS_NOT_COMPATIBLE,
                            "doConversion=" + toString( doConversion_ ) +
                               "newDoConversion=" + toString( newBuf->doConversion() ) );
   }
   if ( stride_ != newBuf->stride() )
   {
      throw E57_EXCEPTION2( E57_ERROR_BUFFERS_NOT_COMPATIBLE,
                            "stride=" + toString( stride_ ) + " newStride=" + toString( newBuf->stride() ) );
   }
}

#ifdef E57_DEBUG
void SourceDestBufferImpl::dump( int indent, std::ostream &os )
{
   /// don't checkImageFileOpen

   os << space( indent ) << "pathName:             " << pathName_ << std::endl;
   os << space( indent ) << "memoryRepresentation: ";
   switch ( memoryRepresentation_ )
   {
      case E57_INT8:
         os << "int8_t" << std::endl;
         break;
      case E57_UINT8:
         os << "uint8_t" << std::endl;
         break;
      case E57_INT16:
         os << "int16_t" << std::endl;
         break;
      case E57_UINT16:
         os << "uint16_t" << std::endl;
         break;
      case E57_INT32:
         os << "int32_t" << std::endl;
         break;
      case E57_UINT32:
         os << "uint32_t" << std::endl;
         break;
      case E57_INT64:
         os << "int64_t" << std::endl;
         break;
      case E57_BOOL:
         os << "bool" << std::endl;
         break;
      case E57_REAL32:
         os << "float" << std::endl;
         break;
      case E57_REAL64:
         os << "double" << std::endl;
         break;
      case E57_USTRING:
         os << "ustring" << std::endl;
         break;
      default:
         os << "<unknown>" << std::endl;
         break;
   }
   os << space( indent ) << "base:                 " << static_cast<const void *>( base_ ) << std::endl;
   os << space( indent ) << "ustrings:             " << static_cast<const void *>( ustrings_ ) << std::endl;
   os << space( indent ) << "capacity:             " << capacity_ << std::endl;
   os << space( indent ) << "doConversion:         " << doConversion_ << std::endl;
   os << space( indent ) << "doScaling:            " << doScaling_ << std::endl;
   os << space( indent ) << "stride:               " << stride_ << std::endl;
   os << space( indent ) << "nextIndex:            " << nextIndex_ << std::endl;
}
#endif

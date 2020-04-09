/***************************************************************************
 *   Copyright (C) 2009,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 *                                                                         *
 *   The dbus-cxx library is free software; you can redistribute it and/or *
 *   modify it under the terms of the GNU General Public License           *
 *   version 3 as published by the Free Software Foundation.               *
 *                                                                         *
 *   The dbus-cxx library is distributed in the hope that it will be       *
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this software. If not see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include <stdint.h>
#include <dbus-cxx/path.h>
#include <dbus-cxx/signatureiterator.h>
#include <any>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <dbus/dbus.h>
#include <stack>
#include "enums.h"

#ifndef DBUSCXX_SIGNATURE_H
#define DBUSCXX_SIGNATURE_H

namespace DBus
{

namespace priv {
    /**
     * Represents a single entry in our graph of the signature
     */
    class SignatureNode {
    public:
        SignatureNode( DataType d ) :
            m_dataType( d ),
            m_next( nullptr ),
            m_sub( nullptr ){}

        DataType m_dataType;
        std::shared_ptr<priv::SignatureNode> m_next;
        std::shared_ptr<priv::SignatureNode> m_sub;
    };
}

  class FileDescriptor;
  class Variant;

  /**
   * Represents a DBus signature.  DBus signatures indicate what type of
   * data the message contains/the method parameters.
   *
   * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
   */
  class Signature
  {
    public:

      typedef SignatureIterator iterator;

      typedef const SignatureIterator const_iterator;

      typedef std::string::size_type size_type;

      static const size_type npos = std::string::npos;

      Signature();

      Signature( const std::string& s, size_type pos = 0, size_type n = npos );

      Signature( const char* );

      Signature( const char* s, size_type n );

      Signature( size_type n, char c );

      template<class InputIterator>
      Signature( InputIterator first, InputIterator last ): m_signature( first, last ) { }

      ~Signature();

      operator const std::string&() const;

      const std::string& str() const;

      Signature& operator=(const std::string& s);

      Signature& operator=(const char* s);

      bool operator==(const std::string& s) const { return m_signature == s; }

      iterator begin();

      const_iterator begin() const;

      iterator end();

      const_iterator end() const;

      /** True if the signature is a valid DBus signature */
      bool is_valid() const;

      /** True if the signature is a valid DBus signature and contains exactly one complete type */
      bool is_singleton() const;

      /**
       * Print the signature tree to the given ostream.
       *
       * @param stream
       */
      void print_tree( std::ostream* stream ) const;

  private:
      void initialize();

      std::shared_ptr<priv::SignatureNode> create_signature_tree( std::string::const_iterator* it,
                                  std::stack<ContainerType>* container_stack,
                                  bool* ok);

      void print_node( std::ostream* stream, priv::SignatureNode* node, int spaces ) const;

    protected:
      std::string m_signature;
      std::shared_ptr<priv::SignatureNode> m_startingNode;
      bool m_valid;

  };

  inline std::string signature( std::any )     { return DBUS_TYPE_INVALID_AS_STRING; }
  inline std::string signature( uint8_t )     { return DBUS_TYPE_BYTE_AS_STRING; }
  inline std::string signature( bool )        { return DBUS_TYPE_BOOLEAN_AS_STRING; }
  inline std::string signature( int16_t )     { return DBUS_TYPE_INT16_AS_STRING; }
  inline std::string signature( uint16_t )    { return DBUS_TYPE_UINT16_AS_STRING; }
  inline std::string signature( int32_t )     { return DBUS_TYPE_INT32_AS_STRING; }
  inline std::string signature( uint32_t )    { return DBUS_TYPE_UINT32_AS_STRING; }
  inline std::string signature( int64_t )     { return DBUS_TYPE_INT64_AS_STRING; }
  inline std::string signature( uint64_t )    { return DBUS_TYPE_UINT64_AS_STRING;      }
  inline std::string signature( double )      { return DBUS_TYPE_DOUBLE_AS_STRING;      }
  inline std::string signature( std::string ) { return DBUS_TYPE_STRING_AS_STRING;      }
  inline std::string signature( Signature )   { return DBUS_TYPE_SIGNATURE_AS_STRING;   }
  inline std::string signature( Path )        { return DBUS_TYPE_OBJECT_PATH_AS_STRING; }
  inline std::string signature( const DBus::Variant& )     { return DBUS_TYPE_VARIANT_AS_STRING; }
  inline std::string signature( const std::shared_ptr<FileDescriptor> )  { return DBUS_TYPE_UNIX_FD_AS_STRING; }

  inline std::string signature( char )        { return DBUS_TYPE_BYTE_AS_STRING;        }
  inline std::string signature( int8_t )      { return DBUS_TYPE_BYTE_AS_STRING;        }
  
#if DBUS_CXX_SIZEOF_LONG_INT == 4
  inline std::string signature( long int )          { return DBUS_TYPE_INT32_AS_STRING;       }
  inline std::string signature( long unsigned int ) { return DBUS_TYPE_UINT32_AS_STRING;      }
#endif
  
  inline std::string signature( float )         { return DBUS_TYPE_DOUBLE_AS_STRING; }
  
   template <typename T> inline std::string signature( const std::vector<T> ) { T t; return DBUS_TYPE_ARRAY_AS_STRING + signature( t ); }

   template <typename Key,typename Data> inline std::string signature( const std::map<Key,Data> )
   {
     Key k; Data d;
     std::string sig;
     sig = DBUS_TYPE_ARRAY_AS_STRING;
     sig += DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING + 
           signature(k) + signature(d) + 
           DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
     return sig;
   }

   //Note: we need to have two different signature() methods for dictionaries; this is because
   //when introspecting, we need to use the normal signature() so that it comes up properly.
   //However, when we are sending out data, that signature would give us an extra array signature,
   //which is not good.  Hence, this method is only used when we need to send out a dict
   template <typename Key,typename Data> inline std::string signature_dict_data( const std::map<Key,Data> )
   {
     Key k; Data d;
     std::string sig;
     sig = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING + 
           signature(k) + signature(d) + 
           DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
     return sig;
   }

  namespace priv {
    /*
     * dbus_signature class - signature of a given type
     */
    template<typename... argn>
    class dbus_signature;
 
    template<> class dbus_signature<>{
    public:
      std::string dbus_sig() const {
        return "";
      }
    };
 
    template<typename arg1, typename... argn>
    class dbus_signature<arg1, argn...> : public dbus_signature<argn...> {
    public:
      std::string dbus_sig() const {
        arg1 arg;
        return signature(arg) + dbus_signature<argn...>::dbus_sig();
      }
    };

  } /* namespace priv */


inline
std::ostream& operator<<(std::ostream& sout, const DBus::Signature& sig)
{
  sout << "DBus::Signature[" << sig.str() << "]";
  return sout;
}

}

#endif

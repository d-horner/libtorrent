// libTorrent - BitTorrent library
// Copyright (C) 2005-2007, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// In addition, as a special exception, the copyright holders give
// permission to link the code of portions of this program with the
// OpenSSL library under certain conditions as described in each
// individual source file, and distribute linked combinations
// including the two.
//
// You must obey the GNU General Public License in all respects for
// all of the code used other than OpenSSL.  If you modify file(s)
// with this exception, you may extend this exception to your version
// of the file(s), but you are not obligated to do so.  If you do not
// wish to do so, delete this exception statement from your version.
// If you delete this exception statement from all source files in the
// program, then also delete it here.
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

#ifndef LIBTORRENT_OBJECT_H
#define LIBTORRENT_OBJECT_H

#include <string>
#include <map>
#include <list>
#include <torrent/common.h>
#include <torrent/exceptions.h>
#include <torrent/object_raw_bencode.h>

namespace torrent {

class LIBTORRENT_EXPORT Object {
public:
  typedef int64_t                           value_type;
  typedef std::string                       string_type;
  typedef std::list<Object>                 list_type;
  typedef std::map<std::string, Object>     map_type;
  typedef map_type::key_type                key_type;

  typedef list_type::iterator               list_iterator;
  typedef list_type::const_iterator         list_const_iterator;
  typedef list_type::reverse_iterator       list_reverse_iterator;
  typedef list_type::const_reverse_iterator list_const_reverse_iterator;

  typedef map_type::iterator                map_iterator;
  typedef map_type::const_iterator          map_const_iterator;
  typedef map_type::reverse_iterator        map_reverse_iterator;
  typedef map_type::const_reverse_iterator  map_const_reverse_iterator;

  typedef std::pair<map_iterator, bool>     map_insert_type;

  // Flags in the range of 0xffff0000 may be set by the user, however
  // 0x00ff0000 are reserved for keywords defined by libtorrent.
  static const uint32_t mask_type     = 0xff;
  static const uint32_t mask_flags    = ~mask_type;
  static const uint32_t mask_internal = 0xffff;
  static const uint32_t mask_public   = ~mask_internal;

  static const uint32_t flag_unordered    = 0x100;    // bencode dictionary was not sorted
  static const uint32_t flag_static_data  = 0x10000;  // Object does not change across sessions.
  static const uint32_t flag_session_data = 0x20000;  // Object changes between sessions.

  enum type_type {
    TYPE_NONE,
    TYPE_RAW_BENCODE,
    TYPE_RAW_VALUE,
    TYPE_RAW_STRING,
    TYPE_RAW_LIST,
    TYPE_RAW_MAP,
    TYPE_VALUE,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_MAP
  };

  Object()                     : m_flags(TYPE_NONE) {}
  Object(const value_type v)   : m_flags(TYPE_VALUE) { new (&_value()) value_type(v); }
  Object(const char* s)        : m_flags(TYPE_STRING) { new (&_string()) string_type(s); }
  Object(const string_type& s) : m_flags(TYPE_STRING) { new (&_string()) string_type(s); }
  Object(const raw_bencode& r) : m_flags(TYPE_RAW_BENCODE) { new (&_raw_bencode()) raw_bencode(r); }
  Object(const raw_value& r)   : m_flags(TYPE_RAW_VALUE) { new (&_raw_value()) raw_value(r); }
  Object(const raw_string& r)  : m_flags(TYPE_RAW_STRING) { new (&_raw_string()) raw_string(r); }
  Object(const raw_list& r)    : m_flags(TYPE_RAW_LIST) { new (&_raw_list()) raw_list(r); }
  Object(const raw_map& r)     : m_flags(TYPE_RAW_MAP) { new (&_raw_map()) raw_map(r); }
  Object(const Object& b);

  ~Object() { clear(); }

  // TODO: Move this out of the class namespace, call them
  // make_object_. 
  static Object       create_empty(type_type t);
  static Object       create_value()  { return Object(value_type()); }
  static Object       create_string() { return Object(string_type()); }
  static Object       create_list()   { Object tmp; tmp.m_flags = TYPE_LIST; new (&tmp._list()) list_type(); return tmp; }
  static Object       create_map()    { Object tmp; tmp.m_flags = TYPE_MAP; new (&tmp._map()) map_type(); return tmp; }

  static Object       create_raw_bencode(raw_bencode obj = raw_bencode()) {
    Object tmp; tmp.m_flags = TYPE_RAW_BENCODE; new (&tmp._raw_bencode()) raw_bencode(); return tmp;
  }
  static Object       create_raw_value(raw_value obj = raw_value()) {
    Object tmp; tmp.m_flags = TYPE_RAW_VALUE; new (&tmp._raw_value()) raw_value(); return tmp;
  }
  static Object       create_raw_string(raw_string obj = raw_string()) {
    Object tmp; tmp.m_flags = TYPE_RAW_STRING; new (&tmp._raw_string()) raw_string(); return tmp;
  }
  static Object       create_raw_list(raw_list obj = raw_list()) {
    Object tmp; tmp.m_flags = TYPE_RAW_LIST; new (&tmp._raw_list()) raw_list(); return tmp;
  }
  static Object       create_raw_map(raw_map obj = raw_map()) {
    Object tmp; tmp.m_flags = TYPE_RAW_MAP; new (&tmp._raw_map()) raw_map(); return tmp;
  }

  // Clear should probably not be inlined due to size and not being
  // optimized away in pretty much any case. Might not work well in
  // cases where we pass constant rvalues.
  void                clear();

  type_type           type() const                            { return (type_type)(m_flags & mask_type); }
  uint32_t            flags() const                           { return m_flags & mask_flags; }

  void                set_flags(uint32_t f)                   { m_flags |= f & mask_public; }
  void                unset_flags(uint32_t f)                 { m_flags &= ~(f & mask_public); }

  void                set_internal_flags(uint32_t f)          { m_flags |= f & (mask_internal & ~mask_type); }
  void                unset_internal_flags(uint32_t f)        { m_flags &= ~(f & (mask_internal & ~mask_type)); }

  // Add functions for setting/clearing the public flags.

  bool                is_empty() const                        { return type() == TYPE_NONE; }
  bool                is_value() const                        { return type() == TYPE_VALUE; }
  bool                is_string() const                       { return type() == TYPE_STRING; }
  bool                is_list() const                         { return type() == TYPE_LIST; }
  bool                is_map() const                          { return type() == TYPE_MAP; }

  value_type&        as_value()                              { check_throw(TYPE_VALUE); return _value(); }
  const value_type&  as_value() const                        { check_throw(TYPE_VALUE); return _value(); }
  string_type&       as_string()                             { check_throw(TYPE_STRING); return _string(); }
  const string_type& as_string() const                       { check_throw(TYPE_STRING); return _string(); }
  list_type&         as_list()                               { check_throw(TYPE_LIST); return _list(); }
  const list_type&   as_list() const                         { check_throw(TYPE_LIST); return _list(); }
  map_type&          as_map()                                { check_throw(TYPE_MAP); return _map(); }
  const map_type&    as_map() const                          { check_throw(TYPE_MAP); return _map(); }
  raw_bencode&       as_raw_bencode()                    { check_throw(TYPE_RAW_BENCODE); return _raw_bencode(); }
  const raw_bencode& as_raw_bencode() const              { check_throw(TYPE_RAW_BENCODE); return _raw_bencode(); }
  raw_value&         as_raw_value()                      { check_throw(TYPE_RAW_VALUE); return _raw_value(); }
  const raw_value&   as_raw_value() const                { check_throw(TYPE_RAW_VALUE); return _raw_value(); }
  raw_string&        as_raw_string()                     { check_throw(TYPE_RAW_STRING); return _raw_string(); }
  const raw_string&  as_raw_string() const               { check_throw(TYPE_RAW_STRING); return _raw_string(); }
  raw_list&          as_raw_list()                       { check_throw(TYPE_RAW_LIST); return _raw_list(); }
  const raw_list&    as_raw_list() const                 { check_throw(TYPE_RAW_LIST); return _raw_list(); }
  raw_map&           as_raw_map()                        { check_throw(TYPE_RAW_MAP); return _raw_map(); }
  const raw_map&     as_raw_map() const                  { check_throw(TYPE_RAW_MAP); return _raw_map(); }

  bool                has_key(const key_type& k) const        { check_throw(TYPE_MAP); return _map().find(k) != _map().end(); }
  bool                has_key_value(const key_type& k) const  { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_VALUE); }
  bool                has_key_string(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_STRING); }
  bool                has_key_list(const key_type& k) const   { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_LIST); }
  bool                has_key_map(const key_type& k) const    { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_MAP); }
  bool                has_key_raw_bencode(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_RAW_BENCODE); }
  bool                has_key_raw_value(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_RAW_VALUE); }
  bool                has_key_raw_string(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_RAW_STRING); }
  bool                has_key_raw_list(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_RAW_LIST); }
  bool                has_key_raw_map(const key_type& k) const { check_throw(TYPE_MAP); return check(_map().find(k), TYPE_RAW_MAP); }

  // Should have an interface for that returns pointer or something,
  // so we don't need to search twice.

  // Make these inline...

  Object&             get_key(const key_type& k);
  const Object&       get_key(const key_type& k) const;
  Object&             get_key(const char* k);
  const Object&       get_key(const char* k) const;

  template <typename T> value_type&        get_key_value(const T& k)        { return get_key(k).as_value(); }
  template <typename T> const value_type&  get_key_value(const T& k) const  { return get_key(k).as_value(); }
  template <typename T> string_type&       get_key_string(const T& k)       { return get_key(k).as_string(); }
  template <typename T> const string_type& get_key_string(const T& k) const { return get_key(k).as_string(); }
  template <typename T> list_type&         get_key_list(const T& k)         { return get_key(k).as_list(); }
  template <typename T> const list_type&   get_key_list(const T& k) const   { return get_key(k).as_list(); }
  template <typename T> map_type&          get_key_map(const T& k)          { return get_key(k).as_map(); }
  template <typename T> const map_type&    get_key_map(const T& k) const    { return get_key(k).as_map(); }

  Object&             insert_key(const key_type& k, const Object& b) { check_throw(TYPE_MAP); return _map()[k] = b; }
  Object&             insert_key_move(const key_type& k, Object& b)  { check_throw(TYPE_MAP); return _map()[k].move(b); }

  // 'insert_preserve_*' inserts the object 'b' if the key 'k' does
  // not exist, else it returns the old entry. The type specific
  // versions also require the old entry to be of the same type.
  //
  // Consider making insert_preserve_* return std::pair<Foo*,bool> or
  // something similar.
  map_insert_type     insert_preserve_any(const key_type& k, const Object& b) { check_throw(TYPE_MAP); return _map().insert(map_type::value_type(k, b)); }
  map_insert_type     insert_preserve_type(const key_type& k, Object& b);
  map_insert_type     insert_preserve_copy(const key_type& k, Object b) { return insert_preserve_type(k, b); }

  void                erase_key(const key_type& k)                   { check_throw(TYPE_MAP); _map().erase(k); }

  Object&             insert_front(const Object& b)                  { check_throw(TYPE_LIST); return *_list().insert(_list().begin(), b); }
  Object&             insert_back(const Object& b)                   { check_throw(TYPE_LIST); return *_list().insert(_list().end(), b); }

  // Copy and merge operations:
  Object&             move(Object& b);
  Object&             swap(Object& b);
  Object&             swap_same_type(Object& b);

  // Only map entries are merged.
  Object&             merge_move(Object& object, uint32_t maxDepth = ~uint32_t());
  Object&             merge_copy(const Object& object,
                                 uint32_t skip_mask = flag_static_data,
                                 uint32_t maxDepth = ~uint32_t());

  Object&             operator = (const Object& b);

  // Internal:
  void                swap_same_type(Object& left, Object& right);

 private:
  // TMP to kill bad uses.
  //  explicit Object(type_type t);

  inline bool         check(map_type::const_iterator itr, type_type t) const { return itr != _map().end() && itr->second.type() == t; }
  inline void         check_throw(type_type t) const                         { if (t != type()) throw bencode_error("Wrong object type."); }

  uint32_t            m_flags;

  value_type&         _value()             { return reinterpret_cast<value_type&>(t_pod); }
  const value_type&   _value() const       { return reinterpret_cast<const value_type&>(t_pod); }
  string_type&        _string()            { return reinterpret_cast<string_type&>(t_string); }
  const string_type&  _string() const      { return reinterpret_cast<const string_type&>(t_string); }
  list_type&          _list()              { return reinterpret_cast<list_type&>(t_list); }
  const list_type&    _list() const        { return reinterpret_cast<const list_type&>(t_list); }
  map_type&           _map()               { return reinterpret_cast<map_type&>(t_map); }
  const map_type&     _map() const         { return reinterpret_cast<const map_type&>(t_map); }
  raw_object&         _raw_object()        { return reinterpret_cast<raw_object&>(t_pod); }
  const raw_object&   _raw_object() const  { return reinterpret_cast<const raw_object&>(t_pod); }
  raw_bencode&        _raw_bencode()       { return reinterpret_cast<raw_bencode&>(t_pod); }
  const raw_bencode&  _raw_bencode() const { return reinterpret_cast<const raw_bencode&>(t_pod); }
  raw_value&          _raw_value()         { return reinterpret_cast<raw_value&>(t_pod); }
  const raw_value&    _raw_value() const   { return reinterpret_cast<const raw_value&>(t_pod); }
  raw_string&         _raw_string()        { return reinterpret_cast<raw_string&>(t_pod); }
  const raw_string&   _raw_string() const  { return reinterpret_cast<const raw_string&>(t_pod); }
  raw_list&           _raw_list()          { return reinterpret_cast<raw_list&>(t_pod); }
  const raw_list&     _raw_list() const    { return reinterpret_cast<const raw_list&>(t_pod); }
  raw_map&            _raw_map()           { return reinterpret_cast<raw_map&>(t_pod); }
  const raw_map&      _raw_map() const     { return reinterpret_cast<const raw_map&>(t_pod); }

  union pod_types {
    value_type t_value;
    char       t_raw_obect[sizeof(raw_object)];
  };

  union {
    pod_types t_pod;
    char    t_string[sizeof(string_type)];
    char    t_list[sizeof(list_type)];
    char    t_map[sizeof(map_type)];
  };
};

inline
Object::Object(const Object& b) {
  m_flags = b.m_flags & (mask_type | mask_public);

  switch (type()) {
  case TYPE_NONE:
  case TYPE_RAW_BENCODE:
  case TYPE_RAW_VALUE: 
  case TYPE_RAW_STRING: 
  case TYPE_RAW_LIST: 
  case TYPE_RAW_MAP:
  case TYPE_VALUE:       t_pod = b.t_pod; break;
  case TYPE_STRING:      new (&_string()) string_type(b._string()); break;
  case TYPE_LIST:        new (&_list()) list_type(b._list()); break;
  case TYPE_MAP:         new (&_map()) map_type(b._map()); break;
  }
}

inline Object
Object::create_empty(type_type t) {
  switch (t) {
  case TYPE_RAW_BENCODE: return create_raw_bencode();
  case TYPE_RAW_VALUE:   return create_raw_value();
  case TYPE_RAW_STRING:  return create_raw_string();
  case TYPE_RAW_LIST:    return create_raw_list();
  case TYPE_RAW_MAP:     return create_raw_map();
  case TYPE_VALUE:       return create_value();
  case TYPE_STRING:      return create_string();
  case TYPE_LIST:        return create_list();
  case TYPE_MAP:         return create_map();
  case TYPE_NONE:
  default: return torrent::Object();
  }
}

inline Object
object_create_raw_bencode_c_str(const char str[]) {
  return Object::create_raw_bencode(raw_bencode(str, strlen(str)));
}

inline void
Object::clear() {
  switch (type()) {
  case TYPE_NONE:
  case TYPE_RAW_BENCODE:
  case TYPE_RAW_VALUE: 
  case TYPE_RAW_STRING: 
  case TYPE_RAW_LIST: 
  case TYPE_RAW_MAP:
  case TYPE_VALUE:  break;
  case TYPE_STRING: _string().~string_type(); break;
  case TYPE_LIST:   _list().~list_type(); break;
  case TYPE_MAP:    _map().~map_type(); break;
  }

  // Only clear type?
  m_flags = TYPE_NONE;
}

// TODO: Don't have separete swap cases for PODs.
inline void
Object::swap_same_type(Object& left, Object& right) {
  std::swap(left.m_flags, right.m_flags);

  switch (left.type()) {
  case Object::TYPE_NONE:
  case Object::TYPE_RAW_BENCODE:
  case Object::TYPE_RAW_VALUE: 
  case Object::TYPE_RAW_STRING: 
  case Object::TYPE_RAW_LIST: 
  case Object::TYPE_RAW_MAP: 
  case Object::TYPE_VALUE:  std::swap(left.t_pod, right.t_pod); break;
  case Object::TYPE_STRING: left._string().swap(right._string()); break;
  case Object::TYPE_LIST:   left._list().swap(right._list()); break;
  case Object::TYPE_MAP:    left._map().swap(right._map()); break;
  }
}

inline void swap(Object& left, Object& right) { left.swap(right); }

inline bool
object_equal(const Object& left, const Object& right) {
  if (left.type() != right.type())
    return false;

  switch (left.type()) {
  case Object::TYPE_NONE:        return true;
    //  case Object::TYPE_RAW_BENCODE: return false;
  case Object::TYPE_VALUE:       return left.as_value() == right.as_value();
  case Object::TYPE_STRING:      return left.as_string() == right.as_string();
//   case Object::TYPE_LIST:   return false;
//   case Object::TYPE_MAP:    return false;
  default: return false;
  }
}

}

#endif

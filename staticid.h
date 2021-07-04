/**
 *   Класс для генерации уникального идентификатора
 */

#pragma once
#include <iostream>
#include <atomic>

/// Класс для генерации уникального идентификатора с именем
template <typename TARGETOBJECT>
class TStaticID
{
  const int m_id;
  const std::string m_name;

public:
  TStaticID() = delete;
  TStaticID( const TStaticID& ) = delete;
  explicit TStaticID( const int& id, const std::string& name ) : m_id(id), m_name(name)  {}
  explicit TStaticID( TStaticID&& id ) : m_id(id.m_id), m_name(std::move(id.m_name))  {}
  TStaticID( const std::string& name): m_id (TStaticID<TARGETOBJECT>::NewID()), m_name(name)  {}
  /// Операторы сравнения
  bool operator==(TStaticID &rhs) const { return m_id == rhs.m_id; }
  bool operator!=(TStaticID &rhs) const { return !(*this == rhs); }

  /// Операторы приведения типов
  operator const int& () const { return m_id; }
  operator const std::string& () const { return m_name; }

  /// Вывод в поток
  friend std::ostream &operator<<(std::ostream &stream, const TStaticID &id) {
    stream << "{" << id.m_name /* << "#" << __.m_id */ << "}";
    return stream;
  }

  static const TStaticID &ErrorID() {
    static TStaticID<TARGETOBJECT> errid({-1}, {"ERROR_ID"});
    return errid;
  }
private:
  /// Генерация нового идентификатора
  static int NewID() {
    static std::atomic<int> newId {0};
    return newId++;
  }
};

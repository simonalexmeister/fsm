/**
 * Реализация конечного автомата
 */

#pragma once

#include "staticid.h"
#include "log.h"

#include <functional>
#include <map>
#include <iostream>
#include <set>
#include <cassert>

//template <typename STATEIDTYPE, typename STATETYPE, typename OBJTYPE, typename EVENTTYPE, typename HANDLEDKEY> class TSimpleSM;
template <typename STATETYPE> class TSimpleSM;
/// Класс состояние объекта
template <typename OBJTYPE, typename EVENTTYPE, typename EVENTKEY>
class TSimpleState  final
{

public:
  using obj_t = OBJTYPE;
  using event_t = EVENTTYPE;
  using eventkey_t = EVENTKEY;
  using this_t = TSimpleState<obj_t, event_t, eventkey_t>;
  using stateid_t = TStaticID<this_t>;
  using sm_t = TSimpleSM<this_t>;

  friend sm_t;
private:
  ///@{ Типы
  using func_t = std::function< this_t& ( obj_t*, const event_t& )>;
  using handlers_t = std::map<eventkey_t, func_t>;
  using onenterfunc_t = std::function< this_t& ( obj_t * )>;
  using onexitfunc_t = std::function<void ( obj_t * )>;
  ///@}
private:
  const stateid_t m_stateid;  ///< ID текущего состояния.
  bool m_ignoreDefaults = false;  ///< флаг игнорирования обработчиков по умолчанию.
  handlers_t m_Handlers;  ///< Обработчики событий в этом состоянии.
public: //protected:
  std::set<eventkey_t> m_Ignore;  ///< Ключи игнорируемых событий в этом состоянии.
  onenterfunc_t OnEnterFunc;  ///< Переопределяемая обработка на входе в состояние.
  onexitfunc_t OnExitFunc;  ///< Переопределяемая обработка на выходе из состояния.

public:

  TSimpleState() = delete;

  TSimpleState(stateid_t &&stateid, bool ignoreDefaults = false)
    : m_stateid(std::move(stateid))
    , m_ignoreDefaults(ignoreDefaults)
    , OnEnterFunc([this]( obj_t*obj) { return std::ref(*this);})
    , OnExitFunc([this]( obj_t*obj) {}) {
    ///\todo Сделать проверку (по ID и строке) на добавление уже существующего состояния.
  }
  bool IgnoreDefaults() { return m_ignoreDefaults; }
  operator const stateid_t &() const { return m_stateid; }
  friend std::ostream &operator<<(std::ostream &stream, this_t &state) {
    stream << state.StateID();
    return stream;
  }
  /// Обработка на входе в состояние
  this_t& Enter(obj_t * obj) { return OnEnterFunc(obj); }
  /// Обработка на выходе
  void Exit(obj_t * obj) { OnExitFunc (obj); }
  /// Идентификатор этого состояния
  const stateid_t& StateID() { return m_stateid; }
  /// Обработка события
  this_t* Handle( obj_t *obj, const event_t& event, eventkey_t key );

  func_t& operator[](eventkey_t key) { return m_Handlers[key]; }
public:// private:
  handlers_t& Handlers() {return m_Handlers;}
};

// Класс конечного автомата
template <typename STATETYPE>
class TSimpleSM
{
public:
  ///@{ Типы
  using state_t = STATETYPE;
  using this_t = TSimpleSM<state_t>;
  using obj_t = typename state_t::obj_t;
  using event_t = typename state_t::event_t;
  using eventkey_t = typename state_t::eventkey_t;
  using states_t = std::map<int, state_t*>;
  ///@}
private:
  state_t* m_curstate;  ///< Указатель на текущее состояние
  state_t& NullState();  ///< Точка входа в машину состояний, начальное состояние
public: // public interface
  explicit TSimpleSM( ): m_curstate (&NullState()) {
    assert(nullptr != &m_curstate);
    m_curstate = &NullState();
  }

  state_t& CurrentState() { return *m_curstate; }  ///< Получить ссылку на текущее состояние
  int Handle(obj_t *obj, const event_t& event, eventkey_t key);  ///< Обработчик событий
  void InitSM(obj_t *obj, state_t &state);  ///< Инициализация машины состояний.

  void NewState( obj_t *obj, state_t &state );  ///< Переход начальное состояние state
  /// Обработчики по умолчанию.
  typename state_t::handlers_t& DefaultHandlers()
    { return NullState().Handlers(); }
protected:
  /// Все состояния конечного автомата
  static states_t& States() { static states_t _states; return _states; }
private:
  /// Передача событий в обработчики по умолчанию
  state_t* HandleDefault( obj_t *obj, const event_t& event, eventkey_t key );
};

/////////////////////// TSimpleState /////////////////////////

template <typename OBJTYPE, typename EVENTTYPE, typename EVENTKEY>
typename TSimpleState<OBJTYPE, EVENTTYPE, EVENTKEY>::this_t*
TSimpleState<OBJTYPE, EVENTTYPE, EVENTKEY>
::Handle(obj_t *obj, const event_t& event, eventkey_t key) {
  // указанные события не обрабатываются
  auto ignore = m_Ignore.find( key );
  if ( ignore != m_Ignore.end( ) )
    return nullptr;

  // Поиск обработчика для события
  auto i = m_Handlers.find( key );
  if ( i == m_Handlers.end( ) )
    return nullptr;

  return &(i->second )(obj, event);
}

/////////////////////// TSimpleSM /////////////////////////

template <typename STATETYPE>
typename TSimpleSM<STATETYPE>::state_t*
TSimpleSM<STATETYPE>
::HandleDefault( obj_t *obj, const event_t& event, eventkey_t key ) {
  if ( m_curstate->IgnoreDefaults() )
   return nullptr;
  auto i = DefaultHandlers().find( key );
  if ( i == DefaultHandlers().end( ) )
   return nullptr;
  return &(i->second )( obj, event );
}

template <typename STATETYPE>
int
TSimpleSM<STATETYPE>
::Handle(obj_t *obj, const event_t& event, eventkey_t key) {
  state_t *newstate = m_curstate->Handle( obj, event, key );
  if((nullptr == newstate) && !(newstate = HandleDefault( obj, event, key )) ) {
    DOUTSM(obj->Descr()<<" Handler not found for event: "<<event<< " in state: "<<*m_curstate);
  } else {
    DOUTSM(obj->Descr()<<" Handled event: "<<event<<" in state:"<<*m_curstate);
    NewState( obj, *newstate );
  }
  return (nullptr==newstate) ? -1 : 0;
}

template <typename STATETYPE>
void
TSimpleSM<STATETYPE>
::NewState( obj_t *obj, state_t &state ) {
  #ifndef NDEBUG
    int count{0}; //?
  #endif
  STATETYPE &oldstate = *m_curstate;
  STATETYPE *newstate = &state;
  assert(nullptr != newstate);
  while (m_curstate != newstate) {
    assert( 3 > count++);  //?
    if (nullptr != m_curstate)
      m_curstate->Exit(obj);
    m_curstate = newstate;
    newstate = &m_curstate->Enter(obj);
  }
  if(&oldstate != m_curstate)
      DOUTSM(obj->Descr()<<" state changed: "<<oldstate<<"->"<<*m_curstate);
}

template <typename STATETYPE>
void
TSimpleSM<STATETYPE>
::InitSM( obj_t *obj, state_t &state ) {
  if (m_curstate != &NullState()) {
    EOUTSM("Wrong initialize state machine for:"<< obj->Descr());
    return;
  }
  NewState(obj, state);
}

template <typename STATETYPE>
typename TSimpleSM<STATETYPE>::state_t &
TSimpleSM<STATETYPE>
::NullState() {
  static state_t nullstate({"NullState"}, true);
  return nullstate;
}

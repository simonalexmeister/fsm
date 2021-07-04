/**
 * Пример использования обобщенного FSM на примере управления виртуальным cd проигрывателем
 */
#include "simplesm.h"

// cd проигрыватель
class Player;
// базовый класс для управляющих событий проигрывателя
class PlayerEvent;

// Тип идентификатора события
using PlayerEventID = TStaticID<PlayerEvent>;
// Базовый класс событий проигрывателя
class PlayerEvent
{
public:
  explicit PlayerEvent(const PlayerEventID &id):m_id(id) {}
  int ID() const { return m_id; }
  friend std::ostream& operator<< (std::ostream& stream, const PlayerEvent& event) {
    stream << event.m_id;
    return stream;
  }
protected:
  const PlayerEventID &m_id;
};

/// Макрос для облегчения конструирования событий
#define DEF_PLAYEREVENT_ID(EVENTDESCRIPTION)                      \
  static const PlayerEventID& Id() {                                                               \
    static PlayerEventID _id { PlayerEventID(EVENTDESCRIPTION) }; \
    return _id;                                                   \
  }

// типы событий
struct play : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("play");
  play() : PlayerEvent(Id()){};
};

struct end_pause : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("end_pause");
  end_pause() : PlayerEvent(Id()){};
};

struct stop : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("stop");
  stop() : PlayerEvent(Id()){};
};

struct pause : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("pause");
  pause() : PlayerEvent(Id()){};
};

struct open_close : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("open_close");
  open_close() : PlayerEvent(Id()){};
};

struct NextSong : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("NextSong");
  NextSong() : PlayerEvent(Id()){};
};

struct PreviousSong : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("PreviousSong");
  PreviousSong() : PlayerEvent(Id()){};
};

// Событие с данными
struct cd_detected : public PlayerEvent
{
  DEF_PLAYEREVENT_ID ("cd_detected");
  cd_detected(std::string name)
      : PlayerEvent(Id())
      , name(name)
  {}
  std::string name;
};

// Тип состояния конечного автомата проигрывателя
using PlayerState = TSimpleState<Player, PlayerEvent, int>;

// Класс проигрывателя
class Player
{
public:
  // Начальная инициализация конечного автомата
  void Start() {
    m_sm.InitSM(this, Empty);
  }
  std::string Descr() { return "Player"; }

  /// Дейстивия на проигрывателем выполняемы из КА
  void start_playback(const PlayerEvent &event) { std::cout << event << "  player::start_playback\n"; }
  void open_drawer(const PlayerEvent &event) { std::cout << event << "  player::open_drawer\n"; }
  void close_drawer(const PlayerEvent &event) { std::cout << event << "  player::close_drawer\n"; }
  void store_cd_info(const PlayerEvent &event) {
    auto ev =  static_cast<const cd_detected &>(event);
    std::cout << event << "  player::store_cd_info, "<< ev.name<<"\n";
  }
  void stop_playback(const PlayerEvent &event) { std::cout << event << "  player::stop_playback\n"; }
  void pause_playback(const PlayerEvent &event) { std::cout << event << "  player::pause_playback\n"; }
  void resume_playback(const PlayerEvent &event) { std::cout << event << "  player::resume_playback\n"; }
  void stop_and_open(const PlayerEvent &event) { std::cout << event << "  player::stop_and_open\n"; }
  void stopped_again(const PlayerEvent &event) {std::cout << event << "  player::stopped_again\n";}

  /// Обработка событий проигрывателя
  int HandleMessage(const PlayerEvent &event) {
    //std::cout << "Handle event: " << event;
    m_sm.Handle( this, event, event.ID());
    return 0;
  }
private:
  /// Конечный автомат проигрывателя
  struct FSM: public TSimpleSM<PlayerState>
  {
    FSM() {
      /// Инициализация КА для всех экземпляров проигрывателя делается один раз
      static int handLersCount = InitStates();
      handLersCount = handLersCount;
    }
  private:
    int InitStates();
  } m_sm;
  // Состояния КА проигрывателя
  static PlayerState Stopped;
  static PlayerState Open;
  static PlayerState Empty;
  static PlayerState Playing;
  static PlayerState Paused;
  /// Получение текущего состояния проигрывателя
  PlayerState& CurrentState() {
    return m_sm.CurrentState();
  }
public:
  /// Печать текущего состояние проигрывателя
  void pstate() {
      std::cout << " -> " << m_sm.CurrentState() << std::endl;
  }
};

// Определение состояний КА
PlayerState Player::Stopped({"Stopped"});
PlayerState Player::Open({"Open"});
PlayerState Player::Empty({"Empty"});
PlayerState Player::Playing({"Playing"});
PlayerState Player::Paused({"Paused"});

// Инициализация конечного автомата проигрывателя
int Player::FSM::InitStates()
{
  // Состояние Stopped
  Stopped.OnEnterFunc = [](Player *player)->PlayerState& {
    std::cout << "entering: Stopped" << std::endl;
    return player->CurrentState();
  };

  Stopped.OnExitFunc = [](Player *player)->PlayerState& {
    std::cout << "leaving: Stopped"  << std::endl;
    return player->CurrentState();
  };

  Stopped[play::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->start_playback(event);
    return Player::Playing;
  };

  Stopped[open_close::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->open_drawer(event);
    return Player::Open;
  };

  Stopped[stop::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->stopped_again(event);
    return Player::Stopped;
  };

  // Состояние Open
  Open.OnEnterFunc = [](Player *player)->PlayerState& {
    std::cout << "entering: Open" << std::endl;
    return player->CurrentState();
  };
  Open.OnExitFunc = [](Player *player)->PlayerState& {
    std::cout << "leaving: Open"  << std::endl;
    return player->CurrentState();
  };
  Open[open_close::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->close_drawer(event);
    return Player::Empty;
  };

  // Состояние Empty
  Empty.OnEnterFunc = [](Player *player)->PlayerState& {
    std::cout << "entering: Empty" << std::endl;
    return player->CurrentState();
  };
  Empty.OnExitFunc = [](Player *player)->PlayerState& {
    std::cout << "leaving: Empty"  << std::endl;
    return player->CurrentState();
  };
  Empty[open_close::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->open_drawer(event);
    return Player::Open;
  };

  Empty[cd_detected::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->store_cd_info(event);
    return Player::Stopped;
  };

  // Состояние Playing
  Playing.OnEnterFunc = [](Player *player)->PlayerState& {
    std::cout << "entering: Playing" << std::endl;
    return player->CurrentState();
  };
  Playing.OnExitFunc = [](Player *player)->PlayerState& {
    std::cout << "leaving: Playing"  << std::endl;
    return player->CurrentState();
  };
  Playing[stop::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->stop_playback(event);
    return Player::Stopped;
  };
  Playing[pause::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->pause_playback(event);
    return Player::Paused;
  };
  Playing[open_close::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->stop_and_open(event);
    return Player::Open;
  };

  // Состояние Paused
  Paused.OnEnterFunc = [](Player *player)->PlayerState& {
    std::cout << "entering: Paused" << std::endl;
    return player->CurrentState();
  };
  Paused.OnExitFunc = [](Player *player)->PlayerState& {
    std::cout << "leaving: Paused"  << std::endl;
    return player->CurrentState();
  };
  Paused[end_pause::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->resume_playback(event);
    return Player::Playing;
  };
  Paused[stop::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->stop_playback(event);
    return Player::Stopped;
  };
  Paused[open_close::Id()] = []( Player *player, const PlayerEvent & event )->PlayerState& {
    player->stop_and_open(event);
    return Player::Open;
  };
  return Stopped.Handlers().size();
}

void test()
{
  Player player;
  // Начало работы конечного автомата, вход в начальное состояние Empty
  player.Start();

  player.HandleMessage(open_close{}); player.pstate();  // Открываем проигрыватель
  player.HandleMessage(open_close()); player.pstate();  // Загружаем cd, закрываем
  player.HandleMessage(cd_detected("louie, louie"));  // Определение альбома
  player.HandleMessage(play());  /// начало проигрывания

  // Проигрывание
  // События для вложенного, нереализованного КА
  player.HandleMessage(NextSong());player.pstate();  //2nd song active
  player.HandleMessage(NextSong());player.pstate();  //3rd song active
  player.HandleMessage(PreviousSong());player.pstate();  //2nd song active

  player.HandleMessage(pause()); player.pstate();  // Пауза

  player.HandleMessage(end_pause());  player.pstate();  // Конец паузы, продолжение проигрывания
  player.HandleMessage(pause()); player.pstate();  // Снова пауза
  player.HandleMessage(stop());  player.pstate();  // Стоп
  // event leading to the same state
  player.HandleMessage(stop());  player.pstate();  // Снова стоп
}

int main(int argc, char const *argv[])
{
   test();
  return 0;
}

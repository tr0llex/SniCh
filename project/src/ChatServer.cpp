#include <Wt/WServer.h>

#include "ChatServer.hpp"


ChatServer::ChatServer(Wt::WServer &server)
        : server_(server) {}

bool ChatServer::connect(ChatServer::Client *client, const User &user, const ChatEventCallback &handleEvent) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (clients_.count(client) == 0) {
        ClientInfo clientInfo;

        clientInfo.sessionId = Wt::WApplication::instance()->sessionId();
        clientInfo.userId = user.getId();
        clientInfo.eventCallback = handleEvent;

        clients_[client] = clientInfo;

        return true;
    }

    return false;
}

bool ChatServer::disconnect(ChatServer::Client *client) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    return clients_.erase(client) == 1;
}

bool ChatServer::signUp(User &user) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (db_.addNewUser(user)) {
        /// Создавать диалог только с самими собой, оповещать никого не надо
        db_.newDialogue(user); //  std::string createDialogue(const User &user, std::string dialogueName) override;

        return true;
    }

    return false;
}

bool ChatServer::login(User &user) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (/* Вызов функции Бориса */db_.isCorrectUser(user)) {
        /// Оповестить только друзей
//        postChatEvent(ChatEvent(ChatEvent::Login, user));

        return true;
    }

    return false;
}

void ChatServer::logout(const User &user) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

//    postChatEvent(ChatEvent(ChatEvent::Logout, user));
}

bool ChatServer::changeProfile(User &user, const User &newUser) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (db_.updateUser(user, newUser)) {

//        postChatEvent(ChatEvent(ChatEvent::UpdateProfile, user));

        return true;
    }

    return false;
}

DialogueList ChatServer::getDialogueList(const User &user) const {
    return db_.getDialogueList(user);
}

Dialogue ChatServer::getDialogue(uint32_t dialogueId) const { // Dialogue.getId()
    return db_.getDialogue(dialogueId); // std::vector<Message> getNMessagesFromDialogue(std::string Dialogue.getId(), long count) override;
}

std::vector<User> ChatServer::getUsersByUserName(const User &findUser) const {
    return db_.getUsersByUserName(findUser);
}

DialogueInfo ChatServer::createDialogue(const User &user, const User &otherUser) { // Dialog createDialogue(std::string firstId, std::string secondId) override; with yourself
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    DialogueInfo dialogueInfo = db_.newDialogue(user, otherUser);

    DialogueInfo dialogueInfoOther(dialogueInfo.getId(), user.getUsername());
    notifyUser(ChatEvent(ChatEvent::NewDialogue, otherUser.getId(), dialogueInfoOther));

    return dialogueInfo;
}

void ChatServer::sendMessage(const User &user, Dialogue &dialogue, Message &message) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    db_.saveMessage(message); // void writeMessage(Message& message) override;
    dialogue.newMessage(message);

    notifyUser(ChatEvent(ChatEvent::NewMessage, user.getId(), DialogueInfo())); // оповестить себя

    for (const auto &participantId : dialogue.getParticipants()) {
        if (!dialogue.withYourself()) {
            notifyUser(ChatEvent(ChatEvent::NewMessage, participantId, dialogue.getInfo(user))); // оповестить получателя
        }
    }
}

#include "unistd.h"

/*void handlerCompilation(std::list<Wt::WString> &resultList, const User &user, const Message &message, const std::wstring &stdIn) {
    sleep(5);
    const std::wstring& stdOut = stdIn;

    /// TODO
    DialogueInfo dialogueInfo(user.getUsername(), message);

    ChatEvent event(ChatEvent::CompilationCode, user.getId(), dialogueInfo, stdOut);

    auto callback = std::bind(client.eventCallback, event);
    server.post(client.sessionId, callback);
}*/
//void ChatServer::runCompilation(const User &user, const Message &message, const std::wstring &stdIn) {
//    std::thread compile(handlerCompilation, resultList_, user, message, stdIn);
//    compile.detach();
//}

void ChatServer::postChatEvent(const ChatEvent &event) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    Wt::WApplication *app = Wt::WApplication::instance();

    for (const auto &i : clients_) {
        if (app && app->sessionId() == i.second.sessionId) {
            i.second.eventCallback(event);
        }
        else {
            auto callback = std::bind(i.second.eventCallback, event);
            server_.post(i.second.sessionId, callback);
        }
    }
}

void ChatServer::notifyUser(const ChatEvent& event) {
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    for (const auto &i : clients_) {
        if (i.second.userId == event.userId_) {
            auto callback = std::bind(i.second.eventCallback, event);
            server_.post(i.second.sessionId, callback);
//            return; TODO оповещаем все сессии пользователя
        }
    }
}

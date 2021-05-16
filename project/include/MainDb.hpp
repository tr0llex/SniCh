#ifndef PROJECT_INCLUDE_MAINDB_HPP_
#define PROJECT_INCLUDE_MAINDB_HPP_

#include <vector>
#include <cassandra.h>
#include "IMainDb.hpp"
#include "Models.hpp"

// getAllMessagesFromDialogue with PAGINATION
// updateMessageIsRead, messageText, messageCode
class MainDb : public IMainDb {
    public:
        MainDb();
        ~MainDb();

        User* searchUserLogin(std::string login, std::string password) override;
        void writeUser(const User& user) override;
        void changePassword(const User& user) override;

        std::string getCodeFromMessage(std::string messageId) override;
        void writeMessage(Message& message) override; // проставить айдишник если не проставлен
        std::vector<Message> getNLastMessagesFromDialogue(std::string dialogueId, long count) override;
        DialogueList getLastNDialoguesWithLastMessage(User user, long count) override;
        std::vector<std::string> getParticipantsLoginsFromDialogue(std::string dialogueId) override;

        std::vector<std::string> getAllDialoguesIdByLogin(std::string login) override;
        std::vector<std::string> getLastNDialoguesIdByLogin(std::string login, long count) override;
        Dialogue createDialogue(std::string firstId, std::string secondId) override;

        void deleteMessage(Message& message) override;
        void deleteDialogue(Dialogue& dialogue) override;

        void connectToDb(const char* contactPoints);
        void disconnectFromDb();

        void migrate();
    private:
        CassFuture* connect_future_;
        CassFuture* close_future_;
        CassCluster* cluster_;
        CassSession* session_;
};

#endif  // PROJECT_INCLUDE_MAINDB_HPP_

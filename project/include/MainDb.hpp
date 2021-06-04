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

        User getUser(std::string login) override;
        bool searchUserLogin(std::string login) const override;
        bool writeUser(const User& user) override;
        void changePassword(const User& user) override;

        std::string getCodeFromMessage(std::string messageId) override;
        void writeMessage(Message& message) override; // проставить айдишник если не проставлен
        void updateDialogueTime(Message& message);
        std::vector<Message> getNLastMessagesFromDialogue(std::string dialogueId, int count) const override;
        DialogueList getLastNDialoguesWithLastMessage(const User& user, int count) const override;
        std::vector<std::string> getParticipantsLoginsFromDialogue(std::string dialogueId) const override;
        void changePaginatedMessages(CassStatement*& statement, int amount, paginatedMessages& messages) override;
        void fillMessagesFromResult(std::vector<Message>& messages, const CassResult* result) const;

        std::vector<std::string> getAllDialoguesIdByLogin(std::string login) override;
        std::vector<std::string> getLastNDialoguesIdByLogin(std::string login, int count) const override;
        void createDialogue(Dialogue& dialogue) override;
        std::string findDialogue(std::vector<std::string> participantsList) const override;
        time_t getTimeLastUpdateFromDialogue(std::string dialogueId, const User& user) const override;

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

#include <Wt/WApplication.h>
#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WLabel.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WVBoxLayout.h>

#include <chrono>

#include "ChatServer.hpp"
#include "DialogueWidget.hpp"
#include "MessageWidget.hpp"
#include "SnippetEditWidget.hpp"

#include "ChatWidget.hpp"


const int kCountLastDialogues = 20;
const int kCountLastMessages = 1000;

static inline int64_t getCurrentTimeMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
}

ChatWidget::ChatWidget(ChatServer &server)
        : WContainerWidget(),
          server_(server),
          loggedIn_(false) {
    auto cookie = Wt::WApplication::instance()->environment().getCookie("userToken");

    if (cookieValidation(cookie)) {
        loggedIn_ = true;

        dialogueList_ = server_.getDialogueList(user_, kCountLastDialogues);

        connect(user_);

        soundMessageReceived_ = std::make_unique<Wt::WSound>("resources/sounds/message_received.mp3");

        startChat();
    } else {
        letLogin();
    }
}

ChatWidget::~ChatWidget() {
    soundMessageReceived_.reset();
    logout();
}

bool ChatWidget::cookieValidation(const std::string *cookie) {
    if (!cookie || cookie->empty()) {
        return false;
    }

    std::string userLogin = server_.verifyToken(*cookie);
    if (userLogin.empty()) {
        return false;
    }

    user_.setUserLogin(userLogin);
    user_.setToken(*cookie);

    return true;
}

void ChatWidget::connect(const User &user) {
    auto handler = std::bind(&ChatWidget::processChatEvent, this, std::placeholders::_1);
    if (server_.connect(this, user, handler)) {
        Wt::WApplication::instance()->enableUpdates(true);
    }
}

void ChatWidget::disconnect() {
    if (server_.disconnect(this)) {
        Wt::WApplication::instance()->enableUpdates(false);
    }
}

void ChatWidget::letSignUp() {
    clear();

    auto vLayout = setLayout(std::make_unique<Wt::WVBoxLayout>());

    auto vContainer = vLayout->addWidget(std::make_unique<Wt::WContainerWidget>());

    vContainer->setStyleClass("auth-block");

    auto vLayout2 = vContainer->setLayout(std::make_unique<Wt::WVBoxLayout>());

    auto image = vLayout2->addWidget(std::make_unique<Wt::WImage>(Wt::WLink("resources/LOGO.png")));
    image->setAlternateText("SNICH");
    image->setStyleClass("snich");
    image->setMaximumSize(400, 500);

    statusMsg_ = vLayout2->addWidget(std::make_unique<Wt::WText>(), 1, Wt::AlignmentFlag::Center);
    statusMsg_->setTextFormat(Wt::TextFormat::Plain);
    statusMsg_->setStyleClass("status-msg");

    auto hLayout = std::make_unique<Wt::WHBoxLayout>();

    hLayout->addWidget(std::make_unique<Wt::WLabel>("Login:"), 1);

    userLoginEdit_ = hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
    userLoginEdit_->setFocus();
    userLoginEdit_->setStyleClass("pass-log");
    auto validatorLogin = std::make_shared<Wt::WLengthValidator>(4, 16);
    validatorLogin->setMandatory(true);
    userLoginEdit_->setValidator(validatorLogin);

    vLayout2->addLayout(std::move(hLayout));


    hLayout = std::make_unique<Wt::WHBoxLayout>();

    hLayout->addWidget(std::make_unique<Wt::WLabel>("Password:"), 1);

    passwordEdit_ = hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
    passwordEdit_->setAttributeValue("type", "password");
    passwordEdit_->setStyleClass("pass-log");


    auto validatorPassword = std::make_shared<Wt::WLengthValidator>(4, 16);
    validatorPassword->setMandatory(true);
    passwordEdit_->setValidator(validatorPassword);


    vLayout2->addLayout(std::move(hLayout));


    hLayout = std::make_unique<Wt::WHBoxLayout>();

    hLayout->addWidget(std::make_unique<Wt::WLabel>("Confirm password:"), 1);

    confirmPasswordEdit_ = hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
    confirmPasswordEdit_->setAttributeValue("type", "password");
    confirmPasswordEdit_->setStyleClass("pass-log");

    auto validatorConfirmPassword = std::make_shared<Wt::WLengthValidator>(4, 16);
    validatorConfirmPassword->setMandatory(true);
    confirmPasswordEdit_->setValidator(validatorConfirmPassword);


    vLayout2->addLayout(std::move(hLayout));

    auto hLayout2 = std::make_unique<Wt::WHBoxLayout>();

    auto signUp = hLayout2->addWidget(std::make_unique<Wt::WPushButton>("Sign up"));

    signUp->setStyleClass("auth-buttons");

    signUp->clicked().connect(this, &ChatWidget::signUp);
    userLoginEdit_->enterPressed().connect(this, [&] {
        passwordEdit_->setFocus();
    });
    passwordEdit_->enterPressed().connect(this, [&] {
        confirmPasswordEdit_->setFocus();
    });
    confirmPasswordEdit_->enterPressed().connect(this, &ChatWidget::signUp);


    auto logIn = hLayout2->addWidget(std::make_unique<Wt::WPushButton>("Log in"));
    logIn->clicked().connect(this, &ChatWidget::letLogin);
    logIn->setStyleClass("auth-buttons");
    logIn->addStyleClass("signup-button");


    vLayout2->addLayout(std::move(hLayout2));

    this->setStyleClass("block-block");
}

void ChatWidget::letLogin() {
    clear();

    auto hPositionalLayout = std::make_unique<Wt::WHBoxLayout>();

    auto vContainer = hPositionalLayout->addWidget(std::make_unique<Wt::WContainerWidget>());

    vContainer->setStyleClass("auth-block");

    auto vLayout = vContainer->setLayout(std::make_unique<Wt::WVBoxLayout>());

    auto image = vLayout->addWidget(std::make_unique<Wt::WImage>(Wt::WLink("resources/LOGO.png")));
    image->setAlternateText("SNICH");
    image->setStyleClass("snich");
    image->setMaximumSize(400, 500);

    auto hLayout = std::make_unique<Wt::WHBoxLayout>();

    statusMsg_ = vLayout->addWidget(std::make_unique<Wt::WText>(), 1, Wt::AlignmentFlag::Center);
    statusMsg_->addStyleClass("status-msg");
    statusMsg_->setTextFormat(Wt::TextFormat::Plain);

    hLayout->addWidget(std::make_unique<Wt::WLabel>("Login:"), 1);

    userLoginEdit_ = hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
    userLoginEdit_->setFocus();

    auto validator = std::make_shared<Wt::WLengthValidator>(4, 16);
    validator->setMandatory(true);
    userLoginEdit_->setValidator(validator);

    userLoginEdit_->setStyleClass("pass-log");

    vLayout->addLayout(std::move(hLayout));

    hLayout = std::make_unique<Wt::WHBoxLayout>();

    hLayout->addWidget(std::make_unique<Wt::WLabel>("Password:"), 1);

    passwordEdit_ = hLayout->addWidget(std::make_unique<Wt::WLineEdit>(), 1);
    passwordEdit_->setAttributeValue("type", "password");
    passwordEdit_->setValidator(validator);

    passwordEdit_->setStyleClass("pass-log");

    auto hLayout2 = std::make_unique<Wt::WHBoxLayout>();

    auto logIn = hLayout2->addWidget(std::make_unique<Wt::WPushButton>("Log in"), 0);
    logIn->setStyleClass("auth-buttons");

    logIn->clicked().connect(this, &ChatWidget::login);
    userLoginEdit_->enterPressed().connect(this, [&] {
        passwordEdit_->setFocus();
    });
    passwordEdit_->enterPressed().connect(this, &ChatWidget::login);


    auto signUp = hLayout2->addWidget(std::make_unique<Wt::WPushButton>("Sign up"), 0);
    signUp->setStyleClass("auth-buttons");
    signUp->addStyleClass("signup-button");
    signUp->clicked().connect(this, &ChatWidget::letSignUp);

    vLayout->addLayout(std::move(hLayout));

    vLayout->addLayout(std::move(hLayout2));

    this->setLayout(std::move(hPositionalLayout));

    this->setStyleClass("block-block");
}

void ChatWidget::startChat() {
    clear();

    this->setStyleClass("chat");

    userLoginEdit_ = nullptr;

    auto dialogueNamePtr = std::make_unique<Wt::WText>();
    auto userNameSearchPtr = std::make_unique<Wt::WLineEdit>();
    auto searchButtonPtr = std::make_unique<Wt::WPushButton>();
    auto endSearchButtonPtr = std::make_unique<Wt::WPushButton>();
    auto snippetButtonPtr = std::make_unique<Wt::WPushButton>();
    auto messagesPtr = std::make_unique<WContainerWidget>();
    auto userListPtr = std::make_unique<WContainerWidget>();
    auto messageEditPtr = std::make_unique<Wt::WTextArea>();
    auto sendButtonPtr = std::make_unique<Wt::WPushButton>();
    auto logoutButtonPtr = std::make_unique<Wt::WPushButton>(" Logout");

    dialogueName_ = dialogueNamePtr.get();
    userNameSearch_ = userNameSearchPtr.get();
    searchButton_ = searchButtonPtr.get();
    endSearchButton_ = endSearchButtonPtr.get();
    snippetButton_ = snippetButtonPtr.get();
    messages_ = messagesPtr.get();
    dialogues_ = userListPtr.get();
    messageEdit_ = messageEditPtr.get();
    sendButton_ = sendButtonPtr.get();
    Wt::Core::observing_ptr<Wt::WPushButton> logoutButton = logoutButtonPtr.get();

    dialogueName_->setStyleClass("chat-dialogue-name");
    messageEdit_->setStyleClass("message-edit");

    searchButton_->addStyleClass("chat-button bi-search");
    endSearchButton_->addStyleClass("chat-button bi-x-lg");
    snippetButton_->addStyleClass("chat-button bi bi-file-earmark-code");
    sendButton_->addStyleClass("chat-button bi bi-arrow-right");
    logoutButton->addStyleClass("chat-button logout-button bi-door-open");

    messageEdit_->setRows(2);

    messages_->setOverflow(Wt::Overflow::Auto);
    dialogues_->setOverflow(Wt::Overflow::Auto);

    createMessengerLayout(std::move(dialogueNamePtr),
                          std::move(userNameSearchPtr),
                          std::move(searchButtonPtr),
                          std::move(endSearchButtonPtr),
                          std::move(snippetButtonPtr),
                          std::move(messagesPtr),
                          std::move(userListPtr),
                          std::move(messageEditPtr),
                          std::move(sendButtonPtr),
                          std::move(logoutButtonPtr));

    clearMessageInput_.setJavaScript
            ("function(o, e) { setTimeout(function() {"
             "" + messageEdit_->jsRef() + ".value='';"
                                          "}, 0); }");
    clearSearchInput_.setJavaScript
            ("function(o, e) { setTimeout(function() {"
             "" + userNameSearch_->jsRef() + ".value='';"
                                             "}, 0); }");

    Wt::WApplication::instance()->setConnectionMonitor(
            "window.monitor={ "
            "'onChange':function(type, newV) {"
            "var connected = window.monitor.status.connectionStatus != 0;"
            "if(connected) {"
            + messageEdit_->jsRef() + ".disabled=false;"
            + messageEdit_->jsRef() + ".placeholder='';"
                                      "} else { "
            + messageEdit_->jsRef() + ".disabled=true;"
            + messageEdit_->jsRef() + ".placeholder='connection lost';"
                                      "}"
                                      "}"
                                      "}");

    Wt::WApplication *app = Wt::WApplication::instance();
    app->addAutoJavaScript("hljs.highlightAll();");

    searchButton_->clicked().connect(this, &ChatWidget::searchUser);
    searchButton_->clicked().connect(clearSearchInput_);

    userNameSearch_->enterPressed().connect(this, &ChatWidget::searchUser);
    userNameSearch_->enterPressed().connect(clearSearchInput_);

    endSearchButton_->clicked().connect(this, &ChatWidget::endSearch);
    endSearchButton_->clicked().connect(clearSearchInput_);

    if (sendButton_) {
        sendButton_->clicked().connect(this, &ChatWidget::sendMessage);
        sendButton_->clicked().connect(clearMessageInput_);
        sendButton_->clicked().connect((Wt::WWidget *) messageEdit_.get(),
                                       &WWidget::setFocus);
    }
    messageEdit_->enterPressed().connect(this, &ChatWidget::sendMessage);
    messageEdit_->enterPressed().connect(clearMessageInput_);
    messageEdit_->enterPressed().connect((WWidget *) messageEdit_.get(),
                                         &WWidget::setFocus);

    auto validator = std::make_shared<Wt::WLengthValidator>(0, 1000);
    messageEdit_->setValidator(validator);

    Wt::WApplication::instance()->setConnectionMonitor(
            "window.monitor={ "
            "'onChange':function(type, newV) {"
            "var connected = window.monitor.status.connectionStatus != 0;"
            "if(connected) {"
            + messageEdit_->jsRef() + ".disabled=false;"
            + messageEdit_->jsRef() + ".placeholder='';"
                                      "} else { "
            + messageEdit_->jsRef() + ".disabled=true;"
            + messageEdit_->jsRef() + ".placeholder='connection lost';"
                                      "}"
                                      "}"
                                      "}"
    );
    messageEdit_->enterPressed().preventDefaultAction();

    snippetButton_->clicked().connect(this, &ChatWidget::editSnippet);

    if (logoutButton) {
        logoutButton->clicked().connect(this, &ChatWidget::logout);
    }

    updateDialogueList();
    blankDialoguePage();
}

void ChatWidget::switchDialogue(const Dialogue &dialogue) {
    messages_->clear();

    editContainer_->show();
    messageEdit_->setFocus();

    currentDialogue_ = dialogue;

    std::vector<Message> messages = server_.getMessagesFromDialogue(dialogue.getId(), kCountLastMessages);
    currentDialogue_.pushMessages(messages);

    dialogueName_->setText(currentDialogue_.getName(user_));

    for (const auto &message : messages) {
        showNewMessage(message);
    }
}

void ChatWidget::logout() {
    if (loggedIn()) {
        loggedIn_ = false;
        currentDialogue_ = Dialogue();
        dialogueList_.clear();
        dialogueName_ = nullptr;

        server_.logout(user_);
        disconnect();

        Wt::WApplication::instance()->removeCookie("userToken");

        letLogin();
    }
}

void ChatWidget::createMessengerLayout(std::unique_ptr<WWidget> dialogueName, std::unique_ptr<WWidget> userNameSearch,
                                       std::unique_ptr<WWidget> searchButton, std::unique_ptr<WWidget> endSearchButton,
                                       std::unique_ptr<WWidget> snippetButton, std::unique_ptr<WWidget> messages,
                                       std::unique_ptr<WWidget> dialogueList, std::unique_ptr<WWidget> messageEdit,
                                       std::unique_ptr<WWidget> sendButton, std::unique_ptr<WWidget> logoutButton) {
    auto vLayout = std::make_unique<Wt::WVBoxLayout>();

    auto hLayout = std::make_unique<Wt::WHBoxLayout>();

    /// <Шапка>
    hLayout->addWidget(std::make_unique<Wt::WText>(tr("projectName")), 0, Wt::AlignmentFlag::Left);
    auto image = hLayout->addWidget(std::make_unique<Wt::WImage>(Wt::WLink("resources/LOGO1.png")));
    image->setAlternateText("SNICH");
    image->setStyleClass("snich-small");
    image->setMaximumSize(50, 20);
    hLayout->addWidget(std::move(logoutButton), 0, Wt::AlignmentFlag::Right);
    vLayout->addLayout(std::move(hLayout), 0);
    /// </Шапка>

    /// <Тело>
    hLayout = std::make_unique<Wt::WHBoxLayout>();

    /// <Левая часть>
    auto vLeftLayout = std::make_unique<Wt::WVBoxLayout>();

    /// <Поиск>
    auto hSearchLayout = std::make_unique<Wt::WHBoxLayout>();
    userNameSearch->setStyleClass("chat-search");
    hSearchLayout->addWidget(std::move(userNameSearch), 1);
    hSearchLayout->addWidget(std::move(searchButton));
    hSearchLayout->addWidget(std::move(endSearchButton));
    vLeftLayout->addLayout(std::move(hSearchLayout));
    /// </Поиск>

    /// <Список диалогов>
    dialogueList->setStyleClass("chat-getDialoguesView");
    vLeftLayout->addWidget(std::move(dialogueList), 1);
    /// </Список диалогов>

    hLayout->addLayout(std::move(vLeftLayout), 0);
    /// </Левая часть>

    /// <Правая часть>
    auto vRightLayout = std::make_unique<Wt::WVBoxLayout>();

    /// <Шапка диалога>
    auto hRightLayout = std::make_unique<Wt::WHBoxLayout>();
    hRightLayout->addWidget(std::move(dialogueName));
    vRightLayout->addLayout(std::move(hRightLayout), 0);
    /// </Шапка диалога>

    /// <Сообщения>
    messages->setStyleClass("chat-msgs");
    vRightLayout->addWidget(std::move(messages), 1);
    /// </Сообщения>

    /// <Поле ввода>
    auto editContainerPtr = std::make_unique<Wt::WContainerWidget>();
    editContainer_ = editContainerPtr.get();

    auto hEditLayout = editContainerPtr->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hEditLayout->addWidget(std::move(snippetButton));
    hEditLayout->addWidget(std::move(messageEdit), 1);
    hEditLayout->addWidget(std::move(sendButton));
    /// </Поле ввода>

    vRightLayout->addWidget(std::move(editContainerPtr));
    /// </Правая часть>

    hLayout->addLayout(std::move(vRightLayout), 1);
    /// </Тело>

    vLayout->addLayout(std::move(hLayout), 1);

    this->setLayout(std::move(vLayout));
}

void ChatWidget::updateDialogueList() {
    if (!dialogues_) {
        return;
    }

    dialogues_->clear();

    for (const auto &dialogue : dialogueList_) {
        auto dialogueWidget = std::make_unique<DialogueWidget>(dialogue.getName(user_), dialogue);
        DialogueWidget *w = dialogues_->addWidget(std::move(dialogueWidget));
        w->setStyleClass("dialogue-block");

        w->setInline(false);

        w->clicked().connect([&] {
            switchDialogue(dialogue);
        });
    }
}

void ChatWidget::blankDialoguePage() {
    currentDialogue_ = Dialogue();

    dialogueName_->setText("");

    messages_->clear();

    auto vLayout = messages_->setLayout(std::make_unique<Wt::WVBoxLayout>());

    auto helpMsg = std::make_unique<Wt::WText>("Select a chat to start messaging");
    helpMsg->setStyleClass("help-msg");
    vLayout->addWidget(std::move(helpMsg), 1, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Middle);

    editContainer_->hide();
}

void ChatWidget::showNewMessage(const Message &message) {
    Wt::WApplication *app = Wt::WApplication::instance();

    bool myMessage = message.getSenderLogin() == user_.getLogin();
    auto messageWidgetPtr = std::make_unique<MessageWidget>(message, myMessage);
    auto messageWidget = messageWidgetPtr.get();

    if (message.isHaveLaunchSnippet()) {
        auto clickedButton = ([=] {
            std::string msg = messageWidget->getInput();
            std::thread t(&ChatServer::runCompilation, &server_, std::ref(server_),
                          user_, message, msg);
            t.detach();
        });

        messageWidget->setClickedRunButton(clickedButton);
    }

    messages_->addWidget(std::move(messageWidgetPtr));

    if (messages_->count() > 1000) {
        messages_->removeChild(messages_->children()[0]);
    }

    app->doJavaScript(messages_->jsRef() + ".scrollTop += "
                      + messages_->jsRef() + ".scrollHeight;");
}

bool ChatWidget::loggedIn() const {
    return loggedIn_;
}

void ChatWidget::signUp() {
    if (loggedIn()) {
        return;
    }

    if (userLoginEdit_->validate() != Wt::ValidationState::Valid ||
        passwordEdit_->validate() != Wt::ValidationState::Valid ||
        confirmPasswordEdit_->validate() != Wt::ValidationState::Valid) {
        statusMsg_->setText("Data is not valid");
        return;
    }

    std::string username = ws2s(userLoginEdit_->text());
    std::string password = ws2s(passwordEdit_->text());
    std::string confirmPassword = ws2s(confirmPasswordEdit_->text());

    if (password != confirmPassword) {
        statusMsg_->setText("Password mismatch");
        return;
    }
    if (password.empty()) {
        statusMsg_->setText("Password field must not be empty");
        return;
    }

    user_ = User(username, password);

    if (server_.createUser(user_, getCurrentTimeMs())) {
        login();
    } else {
        statusMsg_->setText("Sorry, name '" + escapeText(username) +
                            "' is already taken.");
    }
}

void ChatWidget::login() {
    if (loggedIn()) {
        return;
    }

    if (userLoginEdit_->validate() != Wt::ValidationState::Valid ||
        passwordEdit_->validate() != Wt::ValidationState::Valid) {
        statusMsg_->setText("Data is not valid");
        return;
    }

    std::string username = ws2s(userLoginEdit_->text());
    std::string password = ws2s(passwordEdit_->text());

    user_ = User(username, password);

    if (server_.login(user_)) {
        Wt::WApplication::instance()->setCookie("userToken", user_.getToken(), 604800);

        connect(user_);

        loggedIn_ = true;
        dialogueList_ = server_.getDialogueList(user_, kCountLastDialogues);

        if (!soundMessageReceived_) {
            soundMessageReceived_ = std::make_unique<Wt::WSound>("resources/sounds/message_received.mp3");
        }

        startChat();
    } else {
        statusMsg_->setText("Incorrect login or password");
    }
}

void ChatWidget::searchUser() {
    blankDialoguePage();

    if (!userNameSearch_->text().empty()) {
        std::string findUser = ws2s(userNameSearch_->text());

        dialogues_->clear();

        User foundUser = server_.getUserByLogin(findUser);
        if (foundUser.empty()) {
            return;
        }

        Wt::WText *userWidget = dialogues_->addWidget(std::make_unique<Wt::WText>(foundUser.getLogin()));
        userWidget->setStyleClass("dialogue-block");
        userWidget->addStyleClass("dialogue-name");
        if (user_ == foundUser) {
            userWidget->addStyleClass("dialogue-name-self");
        }
        userWidget->setInline(false);

        userWidget->clicked().connect([&, foundUser] {
            for (const auto &dialogue : dialogueList_) {
                if (dialogue.getName(user_) == foundUser.getLogin()) {
                    switchDialogue(dialogue);
                    endSearch();
                    return;
                }
            }

            std::vector<std::string> participantsList;
            participantsList.push_back(user_.getLogin());
            participantsList.push_back(foundUser.getLogin());

            Dialogue dialogue(participantsList, getCurrentTimeMs());

            server_.createDialogue(dialogue);
        });
    }
}

void ChatWidget::endSearch() {
    updateDialogueList();
}

void ChatWidget::sendMessage() {
    std::string messageText;
    bool space = true;
    for (auto ch : ws2s(messageEdit_->text())) {
        if (ch != ' ') {
            space = false;
            messageText += ch;
        } else if (!space) {
            messageText += ch;
        }
    }

    if (currentDialogue_.isEmpty() ||
        messageEdit_->validate() != Wt::ValidationState::Valid ||
        (messageText.empty() && currentSnippet_.empty())) {
        return;
    }

    Message message(currentDialogue_.getId(),
                    user_.getLogin(),
                    messageText,
                    getCurrentTimeMs(),
                    currentSnippet_);

    currentSnippet_.clear();
    snippetButton_->removeStyleClass("bi-file-earmark-code-fill");
    snippetButton_->addStyleClass("bi-file-earmark-code");

    server_.sendMessage(currentDialogue_, message);
}

void ChatWidget::editSnippet() {
    auto dialog = addChild(Wt::cpp14::make_unique<Wt::WDialog>("Write or copy the program"));
    dialog->setStyleClass("edit-snippet-dialog");

    dialog->titleBar()->setStyleClass("title-bar-dialog");

    auto snippetEdit = dialog->contents()->addNew<SnippetEditWidget>(currentSnippet_);

    auto save = dialog->footer()->addNew<Wt::WPushButton>("Save");
    save->setMargin(7, Wt::Side::Right);
    save->addStyleClass("chat-button");
    auto cancel = dialog->footer()->addNew<Wt::WPushButton>("Cancel");
    cancel->addStyleClass("chat-button");
    dialog->rejectWhenEscapePressed();

    save->clicked().connect([=] {
        if (snippetEdit->validate() == Wt::ValidationState::Valid) {
            dialog->accept();
        }
    });
    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->finished().connect([=] {
        if (dialog->result() == Wt::DialogCode::Accepted) {
            currentSnippet_ = snippetEdit->getSnippet();
            if (!currentSnippet_.empty()) {
                snippetButton_->removeStyleClass("bi-file-earmark-code");
                snippetButton_->addStyleClass("bi-file-earmark-code-fill");
            }
        } else {
            currentSnippet_.clear();
            snippetButton_->removeStyleClass("bi-file-earmark-code-fill");
            snippetButton_->addStyleClass("bi-file-earmark-code");
        }

        removeChild(dialog);

        messageEdit_->setFocus();
    });

    dialog->show();
}

void ChatWidget::processChatEvent(const ChatEvent &event) {
    Wt::WApplication *app = Wt::WApplication::instance();

    app->triggerUpdate();

    switch (event.type()) {
        case ChatEvent::kNewDialogue: {
            dialogueList_.insert(event.dialogue());
            updateDialogueList();

            if (soundMessageReceived_) {
                soundMessageReceived_->play();
            }
            break;
        }
        case ChatEvent::kNewMessage: {
            Dialogue updatedDialogue;
            for (const auto &dialogue : dialogueList_) {
                if (dialogue == event.dialogue()) {
                    updatedDialogue = dialogue;
                    updatedDialogue.updateLastMessage(event.dialogue().getLastMessage());
                    dialogueList_.erase(dialogue);
                    break;
                }
            }
            dialogueList_.insert(updatedDialogue);
            updateDialogueList();

            if (currentDialogue_ == event.dialogue()) {
                showNewMessage(event.dialogue().getLastMessage());
            }

            if (soundMessageReceived_ && user_.getLogin() != event.getSenderLogin()) {
                soundMessageReceived_->play();
            }

            break;
        }
        case ChatEvent::kCompilationCode: {
            if (event.getDialogueId() != currentDialogue_.getId()) {
                break;
            }

            for (int i = 0; i < messages_->count(); ++i) {
                auto messageWidget = dynamic_cast<MessageWidget *>(messages_->widget(i));
                if (messageWidget->getMessageId() == event.getMessageId() && messageWidget->isHaveSnippet()) {
                    messageWidget->setResultCompilation(event.resultCompilation());
                    break;
                }
            }

            break;
        }
        default:
            break;
    }
}

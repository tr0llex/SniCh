#include "DialogueWidget.hpp"

const int kCountSymbolLastMessage = 20;

static inline Wt::WString lastMessageView(const Message &message) {
    Wt::WString messageStr(message.getMessageText());

    if (messageStr.toUTF16().size() < kCountSymbolLastMessage) {
        return messageStr;
    }

    messageStr = messageStr.toUTF16().substr(0, kCountSymbolLastMessage);
    messageStr += Wt::WString(" ...");

    return messageStr;
}

DialogueWidget::DialogueWidget(const std::string &dialogueName, const Dialogue &dialogue)
: dialogue_(dialogue) {
    auto dialogueNamePtr = std::make_unique<Wt::WText>(dialogueName);
    Wt::WString lastMessageStr = lastMessageView(dialogue.getLastMessage());
    auto lastMessagePtr = std::make_unique<Wt::WText>(lastMessageStr);
    auto timePtr = std::make_unique<Wt::WText>(dialogue.getTimeOfLastUpdateStr());

    dialogueName_ = dialogueNamePtr.get();
    lastMessage_ = lastMessagePtr.get();
    time_ = timePtr.get();

    createLayout(std::move(dialogueNamePtr),
                 std::move(lastMessagePtr),
                 std::move(timePtr));
}

void DialogueWidget::createLayout(std::unique_ptr<WWidget> dialogueName, std::unique_ptr<WWidget> lastMessage,
                                  std::unique_ptr<WWidget> time) {
    auto vLayout = std::make_unique<Wt::WVBoxLayout>();

    auto hLayout = std::make_unique<Wt::WHBoxLayout>();

    dialogueName->setStyleClass("dialogue-name");
    if (dialogue_.withYourself()) {
        dialogueName->addStyleClass("dialogue-name-self");
    }
    hLayout->addWidget(std::move(dialogueName), 1);
    time->setStyleClass("chat-time");
    hLayout->addWidget(std::move(time));
    vLayout->addLayout(std::move(hLayout), 0);

    hLayout = std::make_unique<Wt::WHBoxLayout>();
    hLayout->addWidget(std::move(lastMessage), 0, Wt::AlignmentFlag::Left);
    if (dialogue_.getLastMessage().isHaveSnippet()) {
        auto snippetIcon = std::make_unique<Wt::WText>();
        snippetIcon->addStyleClass("snippet-in-dialogue-list bi-file-earmark-code-fill");
        hLayout->addWidget(std::move(snippetIcon), 1, Wt::AlignmentFlag::Left);
    }
    vLayout->addLayout(std::move(hLayout), 0);

    this->setLayout(std::move(vLayout));
}

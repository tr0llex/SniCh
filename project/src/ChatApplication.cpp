#include "ChatApplication.hpp"

ChatApplication::ChatApplication(const Wt::WEnvironment& env, ChatServer& server)
        : WApplication(env),
          server_(server),
          env_(env) {
    setTitle("SnippetChat");
    useStyleSheet("resources/chatapp.css");
    useStyleSheet("resources/highlight/styles/atom-one-dark.css");
    require("resources/highlight/highlight.pack.js");

    messageResourceBundle().use(appRoot() + "resources/SnippetChat");
    
    root()->addWidget(std::make_unique<ChatWidget>(server_));
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment &env, ChatServer &server) {
    return std::make_unique<ChatApplication>(env, server);
}

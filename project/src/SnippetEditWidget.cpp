#include <Wt/WComboBox.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLabel.h>
#include <Wt/WTextArea.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WLengthValidator.h>

#include "SnippetEditWidget.hpp"


static inline int convertLanguageToIndex(Snippet::Language language) {
    return language;
}

static inline Snippet::Language convertIndexToLanguage(int index) {
    return Snippet::Language(index);
}

SnippetEditWidget::SnippetEditWidget(const Snippet &snippet)
: snippet_(snippet) {
    auto switchLanguagePtr = std::make_unique<Wt::WComboBox>();
    auto editCodePtr = std::make_unique<Wt::WTextArea>(snippet.getProgramText());

    switchLanguage_ = switchLanguagePtr.get();
    editCode_ = editCodePtr.get();

    auto labelPtr = std::make_unique<Wt::WLabel>("Language: ");
    labelPtr->setBuddy(switchLanguage_);

    switchLanguage_->addItem("Python 3");
    switchLanguage_->addItem("C++ 14");
    switchLanguage_->addItem("C++ 17");
    switchLanguage_->addItem("C++ 20");
    switchLanguage_->addItem("C 17");
    switchLanguage_->addItem("Another");

    if (snippet_.empty()) {
        snippet_.setLanguage(Snippet::Language::kAnother);
    }
    switchLanguage_->setCurrentIndex(convertLanguageToIndex(snippet_.getLanguage()));

    auto validator = std::make_shared<Wt::WLengthValidator>(1, 10000);
    editCode_->setValidator(validator);
    editCode_->setRows(30);
    editCode_->setWidth(700);
    editCode_->setFocus();

    createLayout(std::move(labelPtr),
                 std::move(switchLanguagePtr),
                 std::move(editCodePtr));

    switchLanguage_->changed().connect(this, &SnippetEditWidget::updateLanguage);
}

void SnippetEditWidget::createLayout(std::unique_ptr<WWidget> label,
                                     std::unique_ptr<WWidget> switchLanguage,
                                     std::unique_ptr<WWidget> editCode) {
    auto vLayout = std::make_unique<Wt::WVBoxLayout>();

    auto hLayout = std::make_unique<Wt::WHBoxLayout>();
    hLayout->addWidget(std::move(label));
    hLayout->addWidget(std::move(switchLanguage));
    vLayout->addLayout(std::move(hLayout), 0, Wt::AlignmentFlag::Right);
    editCode->setStyleClass("edit-code");
    vLayout->addWidget(std::move(editCode));

    this->setLayout(std::move(vLayout));
}

Snippet SnippetEditWidget::getSnippet() {
    snippet_.setProgramText(ws2s(editCode_->text()));
    return snippet_;
}

void SnippetEditWidget::updateLanguage() {
    snippet_.setLanguage(convertIndexToLanguage(switchLanguage_->currentIndex()));
}

Wt::ValidationState SnippetEditWidget::validate() const {
    return editCode_->validate();
}

#include "LoginDialog.h"
#include <Wt/WBreak.h>
#include <Wt/WLabel.h>
#include <Wt/WImage.h>
#include <Wt/WApplication.h>
#include <iostream>

namespace FranchiseAI {
namespace Widgets {

LoginDialog::LoginDialog()
    : Wt::WDialog("Login")
{
    authService_ = std::make_unique<Services::AuthService>();
    setupUI();
}

void LoginDialog::setupUI() {
    // Configure dialog
    setModal(true);
    setClosable(false);  // User must log in
    setResizable(false);
    setTitleBarEnabled(false);  // Hide title bar for cleaner look

    // Add custom CSS class for styling
    addStyleClass("login-dialog");

    // Get the contents container
    auto* contents = this->contents();
    contents->addStyleClass("login-dialog-content");

    // Logo/Brand section
    auto* brandContainer = contents->addWidget(std::make_unique<Wt::WContainerWidget>());
    brandContainer->addStyleClass("login-brand");

    auto* logo = brandContainer->addWidget(std::make_unique<Wt::WText>("FranchiseAI"));
    logo->addStyleClass("login-logo");

    auto* tagline = brandContainer->addWidget(std::make_unique<Wt::WText>("Prospect Discovery Platform"));
    tagline->addStyleClass("login-tagline");

    // Form container
    auto* formContainer = contents->addWidget(std::make_unique<Wt::WContainerWidget>());
    formContainer->addStyleClass("login-form");

    // Error message container (hidden by default)
    errorContainer_ = formContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    errorContainer_->addStyleClass("login-error");
    errorContainer_->hide();

    errorMessage_ = errorContainer_->addWidget(std::make_unique<Wt::WText>());
    errorMessage_->addStyleClass("login-error-text");

    // Email field
    auto* emailGroup = formContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    emailGroup->addStyleClass("form-group");

    auto* emailLabel = emailGroup->addWidget(std::make_unique<Wt::WLabel>("Email"));
    emailInput_ = emailGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    emailInput_->setPlaceholderText("Enter your email");
    emailInput_->addStyleClass("form-control");
    emailLabel->setBuddy(emailInput_);

    // Password field
    auto* passwordGroup = formContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    passwordGroup->addStyleClass("form-group");

    auto* passwordLabel = passwordGroup->addWidget(std::make_unique<Wt::WLabel>("Password"));
    passwordInput_ = passwordGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    passwordInput_->setPlaceholderText("Enter your password");
    passwordInput_->setEchoMode(Wt::EchoMode::Password);
    passwordInput_->addStyleClass("form-control");
    passwordLabel->setBuddy(passwordInput_);

    // Remember me checkbox
    auto* rememberGroup = formContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    rememberGroup->addStyleClass("form-group-checkbox");

    rememberMe_ = rememberGroup->addWidget(std::make_unique<Wt::WCheckBox>("Remember me"));
    rememberMe_->addStyleClass("form-check-input");

    // Login button
    auto* buttonGroup = formContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    buttonGroup->addStyleClass("form-group-button");

    loginButton_ = buttonGroup->addWidget(std::make_unique<Wt::WPushButton>("Sign In"));
    loginButton_->addStyleClass("btn btn-primary btn-block login-button");
    loginButton_->clicked().connect(this, &LoginDialog::onLoginClicked);

    // Enter key triggers login
    emailInput_->enterPressed().connect(this, &LoginDialog::onLoginClicked);
    passwordInput_->enterPressed().connect(this, &LoginDialog::onLoginClicked);

    // Footer with help text
    auto* footer = contents->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer->addStyleClass("login-footer");

    auto* helpText = footer->addWidget(std::make_unique<Wt::WText>(
        "Default credentials: admin@franchiseai.com / admin123"
    ));
    helpText->addStyleClass("login-help-text");

    // Add inline styles for the dialog
    std::string inlineStyles = R"CSS(
        .login-dialog {
            width: 400px !important;
            max-width: 90vw;
            border-radius: 12px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            border: none;
        }
        .login-dialog-content {
            padding: 40px;
            background: white;
            border-radius: 12px;
        }
        .login-brand {
            text-align: center;
            margin-bottom: 30px;
        }
        .login-logo {
            display: block;
            font-size: 28px;
            font-weight: 700;
            color: #2563eb;
            margin-bottom: 8px;
        }
        .login-tagline {
            display: block;
            font-size: 14px;
            color: #6b7280;
        }
        .login-form {
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        .form-group label {
            display: block;
            margin-bottom: 6px;
            font-weight: 500;
            color: #374151;
            font-size: 14px;
        }
        .form-control {
            width: 100%;
            padding: 12px 14px;
            border: 1px solid #d1d5db;
            border-radius: 8px;
            font-size: 14px;
            transition: border-color 0.2s, box-shadow 0.2s;
        }
        .form-control:focus {
            outline: none;
            border-color: #2563eb;
            box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);
        }
        .form-group-checkbox {
            margin-bottom: 24px;
        }
        .form-group-button {
            margin-bottom: 0;
        }
        .login-button {
            width: 100%;
            padding: 12px;
            font-size: 16px;
            font-weight: 600;
            border-radius: 8px;
            background: #2563eb;
            border: none;
            color: white;
            cursor: pointer;
            transition: background 0.2s;
        }
        .login-button:hover {
            background: #1d4ed8;
        }
        .login-button:disabled {
            background: #9ca3af;
            cursor: not-allowed;
        }
        .login-error {
            background: #fef2f2;
            border: 1px solid #fecaca;
            border-radius: 8px;
            padding: 12px;
            margin-bottom: 20px;
        }
        .login-error-text {
            color: #dc2626;
            font-size: 14px;
        }
        .login-footer {
            text-align: center;
            padding-top: 20px;
            border-top: 1px solid #e5e7eb;
        }
        .login-help-text {
            font-size: 12px;
            color: #9ca3af;
        }
    )CSS";

    Wt::WApplication::instance()->styleSheet().addRule(".login-dialog", "width: 400px !important; max-width: 90vw; border-radius: 12px; box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3); border: none;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-dialog-content", "padding: 40px; background: white; border-radius: 12px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-brand", "text-align: center; margin-bottom: 30px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-logo", "display: block; font-size: 28px; font-weight: 700; color: #2563eb; margin-bottom: 8px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-tagline", "display: block; font-size: 14px; color: #6b7280;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-form", "margin-bottom: 20px;");
    Wt::WApplication::instance()->styleSheet().addRule(".form-group", "margin-bottom: 20px;");
    Wt::WApplication::instance()->styleSheet().addRule(".form-group label", "display: block; margin-bottom: 6px; font-weight: 500; color: #374151; font-size: 14px;");
    Wt::WApplication::instance()->styleSheet().addRule(".form-control", "width: 100%; padding: 12px 14px; border: 1px solid #d1d5db; border-radius: 8px; font-size: 14px;");
    Wt::WApplication::instance()->styleSheet().addRule(".form-group-checkbox", "margin-bottom: 24px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-button", "width: 100%; padding: 12px; font-size: 16px; font-weight: 600; border-radius: 8px; background: #2563eb; border: none; color: white;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-button:hover", "background: #1d4ed8;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-error", "background: #fef2f2; border: 1px solid #fecaca; border-radius: 8px; padding: 12px; margin-bottom: 20px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-error-text", "color: #dc2626; font-size: 14px;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-footer", "text-align: center; padding-top: 20px; border-top: 1px solid #e5e7eb;");
    Wt::WApplication::instance()->styleSheet().addRule(".login-help-text", "font-size: 12px; color: #9ca3af;");
}

void LoginDialog::onLoginClicked() {
    hideError();

    std::string email = emailInput_->text().toUTF8();
    std::string password = passwordInput_->text().toUTF8();

    // Validate input
    if (email.empty()) {
        showError("Please enter your email address");
        emailInput_->setFocus();
        return;
    }

    if (password.empty()) {
        showError("Please enter your password");
        passwordInput_->setFocus();
        return;
    }

    // Basic email format validation
    if (email.find('@') == std::string::npos) {
        showError("Please enter a valid email address");
        emailInput_->setFocus();
        return;
    }

    // Show loading state
    setLoading(true);

    // Attempt login
    std::cout << "[LoginDialog] Attempting login for: " << email << std::endl;

    // Get client IP (if available)
    std::string ipAddress;
    auto* app = Wt::WApplication::instance();
    if (app && app->environment().clientAddress().length() > 0) {
        ipAddress = app->environment().clientAddress();
    }

    Services::LoginResult result = authService_->login(email, password, ipAddress);

    setLoading(false);

    if (result.success) {
        std::cout << "[LoginDialog] Login successful, emitting signal" << std::endl;
        loginSuccessful_.emit(result);
        accept();  // Close the dialog
    } else {
        std::cout << "[LoginDialog] Login failed: " << result.errorMessage << std::endl;
        showError(result.errorMessage);
        passwordInput_->setText("");
        passwordInput_->setFocus();
    }
}

void LoginDialog::validateInput() {
    bool valid = !emailInput_->text().empty() && !passwordInput_->text().empty();
    loginButton_->setEnabled(valid);
}

void LoginDialog::showError(const std::string& message) {
    errorMessage_->setText(message);
    errorContainer_->show();
}

void LoginDialog::hideError() {
    errorContainer_->hide();
}

void LoginDialog::setLoading(bool loading) {
    if (loading) {
        loginButton_->setText("Signing in...");
        loginButton_->setEnabled(false);
        emailInput_->setEnabled(false);
        passwordInput_->setEnabled(false);
    } else {
        loginButton_->setText("Sign In");
        loginButton_->setEnabled(true);
        emailInput_->setEnabled(true);
        passwordInput_->setEnabled(true);
    }
}

void LoginDialog::reset() {
    emailInput_->setText("");
    passwordInput_->setText("");
    rememberMe_->setChecked(false);
    hideError();
    setLoading(false);
}

void LoginDialog::focusEmail() {
    emailInput_->setFocus();
}

} // namespace Widgets
} // namespace FranchiseAI

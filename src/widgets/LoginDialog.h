#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>
#include "../services/AuthService.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Modal login dialog for user authentication
 *
 * Displays a centered login form with:
 * - Email input field
 * - Password input field
 * - Remember me checkbox
 * - Login button
 * - Error message display
 */
class LoginDialog : public Wt::WDialog {
public:
    LoginDialog();
    ~LoginDialog() override = default;

    /**
     * @brief Signal emitted when login is successful
     * @return Signal with LoginResult containing user info and session token
     */
    Wt::Signal<Services::LoginResult>& loginSuccessful() { return loginSuccessful_; }

    /**
     * @brief Reset the form to initial state
     */
    void reset();

    /**
     * @brief Set focus to email input
     */
    void focusEmail();

private:
    void setupUI();
    void onLoginClicked();
    void validateInput();
    void showError(const std::string& message);
    void hideError();
    void setLoading(bool loading);

    // UI components
    Wt::WLineEdit* emailInput_ = nullptr;
    Wt::WLineEdit* passwordInput_ = nullptr;
    Wt::WCheckBox* rememberMe_ = nullptr;
    Wt::WPushButton* loginButton_ = nullptr;
    Wt::WText* errorMessage_ = nullptr;
    Wt::WContainerWidget* errorContainer_ = nullptr;

    // Services
    std::unique_ptr<Services::AuthService> authService_;

    // Signals
    Wt::Signal<Services::LoginResult> loginSuccessful_;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // LOGIN_DIALOG_H

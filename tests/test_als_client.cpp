// ============================================================================
// ApiLogicServer Client Test Cases
// Tests for StoreLocation and Franchisee CRUD operations
// ============================================================================

#include <iostream>
#include <cassert>
#include <string>
#include "../src/services/ApiLogicServerClient.h"

using namespace FranchiseAI::Services;

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    if (condition) { \
        std::cout << "  ✓ PASS: " << message << std::endl; \
        tests_passed++; \
    } else { \
        std::cout << "  ✗ FAIL: " << message << std::endl; \
        tests_failed++; \
    }

// ============================================================================
// Test Case 1: Retrieve Current StoreLocation and Franchisee
// ============================================================================
void test_retrieve_current_store_and_franchisee(ApiLogicServerClient& client) {
    std::cout << "\n=== Test Case 1: Retrieve Current StoreLocation and Franchisee ===" << std::endl;

    // Load all AppConfig entries into cache
    client.loadAppConfigs();

    // Get current_franchisee_id from AppConfig
    std::string franchiseeId = client.getAppConfigValue("current_franchisee_id");
    TEST_ASSERT(!franchiseeId.empty(), "current_franchisee_id exists in AppConfig");
    std::cout << "    current_franchisee_id = " << franchiseeId << std::endl;

    // Get current_store_id from AppConfig
    std::string storeId = client.getAppConfigValue("current_store_id");
    TEST_ASSERT(!storeId.empty(), "current_store_id exists in AppConfig");
    std::cout << "    current_store_id = " << storeId << std::endl;

    // Retrieve Franchisee by ID
    if (!franchiseeId.empty()) {
        auto response = client.getFranchisee(franchiseeId);
        TEST_ASSERT(response.success, "Franchisee retrieved successfully");

        if (response.success) {
            auto franchisee = FranchiseeDTO::fromJson(response.body);
            TEST_ASSERT(!franchisee.id.empty(), "Franchisee has valid ID");
            TEST_ASSERT(franchisee.id == franchiseeId, "Franchisee ID matches requested ID");
            std::cout << "    Franchisee: " << franchisee.businessName << std::endl;
        }
    }

    // Retrieve StoreLocation by ID
    if (!storeId.empty()) {
        auto response = client.getStoreLocation(storeId);
        TEST_ASSERT(response.success, "StoreLocation retrieved successfully");

        if (response.success) {
            auto store = StoreLocationDTO::fromJson(response.body);
            TEST_ASSERT(!store.id.empty(), "StoreLocation has valid ID");
            TEST_ASSERT(store.id == storeId, "StoreLocation ID matches requested ID");
            std::cout << "    Store: " << store.storeName << " at " << store.city << ", " << store.stateProvince << std::endl;
        }
    }
}

// ============================================================================
// Test Case 2: PATCH Current StoreLocation and Franchisee
// ============================================================================
void test_patch_store_and_franchisee(ApiLogicServerClient& client) {
    std::cout << "\n=== Test Case 2: PATCH Current StoreLocation and Franchisee ===" << std::endl;

    // Get current IDs from AppConfig
    std::string franchiseeId = client.getAppConfigValue("current_franchisee_id");
    std::string storeId = client.getAppConfigValue("current_store_id");

    // --- PATCH Franchisee ---
    if (!franchiseeId.empty()) {
        // First retrieve current data
        auto getResponse = client.getFranchisee(franchiseeId);
        TEST_ASSERT(getResponse.success, "Retrieved Franchisee for PATCH");

        if (getResponse.success) {
            auto franchisee = FranchiseeDTO::fromJson(getResponse.body);
            std::string originalName = franchisee.businessName;

            // Modify a field
            franchisee.businessName = "Updated Franchise Name - Test";
            franchisee.id = franchiseeId;  // Ensure ID is set for PATCH

            // Save (should PATCH since ID is set)
            auto patchResponse = client.saveFranchisee(franchisee);
            TEST_ASSERT(patchResponse.success, "Franchisee PATCH successful");
            TEST_ASSERT(patchResponse.statusCode == 200 || patchResponse.statusCode == 204,
                       "Franchisee PATCH returned 200/204 status");

            // Verify the change
            auto verifyResponse = client.getFranchisee(franchiseeId);
            if (verifyResponse.success) {
                auto updated = FranchiseeDTO::fromJson(verifyResponse.body);
                TEST_ASSERT(updated.businessName == "Updated Franchise Name - Test",
                           "Franchisee businessName was updated");
            }

            // Restore original value
            franchisee.businessName = originalName;
            client.saveFranchisee(franchisee);
            std::cout << "    Restored original Franchisee name: " << originalName << std::endl;
        }
    }

    // --- PATCH StoreLocation ---
    if (!storeId.empty()) {
        // First retrieve current data
        auto getResponse = client.getStoreLocation(storeId);
        TEST_ASSERT(getResponse.success, "Retrieved StoreLocation for PATCH");

        if (getResponse.success) {
            auto store = StoreLocationDTO::fromJson(getResponse.body);
            std::string originalName = store.storeName;

            // Modify a field
            store.storeName = "Updated Store Name - Test";
            store.id = storeId;  // Ensure ID is set for PATCH

            // Save (should PATCH since ID is set)
            auto patchResponse = client.saveStoreLocation(store);
            TEST_ASSERT(patchResponse.success, "StoreLocation PATCH successful");
            TEST_ASSERT(patchResponse.statusCode == 200 || patchResponse.statusCode == 204,
                       "StoreLocation PATCH returned 200/204 status");

            // Verify the change
            auto verifyResponse = client.getStoreLocation(storeId);
            if (verifyResponse.success) {
                auto updated = StoreLocationDTO::fromJson(verifyResponse.body);
                TEST_ASSERT(updated.storeName == "Updated Store Name - Test",
                           "StoreLocation storeName was updated");
            }

            // Restore original value
            store.storeName = originalName;
            client.saveStoreLocation(store);
            std::cout << "    Restored original StoreLocation name: " << originalName << std::endl;
        }
    }
}

// ============================================================================
// Test Case 3: POST New StoreLocation and Franchisee, Update AppConfig
// ============================================================================
void test_post_new_store_and_franchisee(ApiLogicServerClient& client) {
    std::cout << "\n=== Test Case 3: POST New StoreLocation and Franchisee ===" << std::endl;

    // Save original AppConfig values to restore later
    std::string originalFranchiseeId = client.getAppConfigValue("current_franchisee_id");
    std::string originalStoreId = client.getAppConfigValue("current_store_id");

    // --- POST New Franchisee ---
    FranchiseeDTO newFranchisee;
    newFranchisee.id = "";  // Empty ID triggers POST with UUID generation
    newFranchisee.businessName = "Test Franchise - POST Test";
    newFranchisee.ownerFirstName = "Test";
    newFranchisee.ownerLastName = "Owner";
    newFranchisee.email = "test@example.com";
    newFranchisee.phone = "(555) 123-4567";
    newFranchisee.addressLine1 = "123 Test Street";
    newFranchisee.city = "Test City";
    newFranchisee.stateProvince = "TS";
    newFranchisee.postalCode = "12345";
    newFranchisee.latitude = 40.0;
    newFranchisee.longitude = -105.0;
    newFranchisee.isActive = true;

    auto franchiseeResponse = client.saveFranchisee(newFranchisee);
    TEST_ASSERT(franchiseeResponse.success, "Franchisee POST successful");
    TEST_ASSERT(franchiseeResponse.statusCode == 201 || franchiseeResponse.statusCode == 200,
               "Franchisee POST returned 201/200 status");

    std::string newFranchiseeId;
    if (franchiseeResponse.success) {
        auto created = FranchiseeDTO::fromJson(franchiseeResponse.body);
        newFranchiseeId = created.id;
        TEST_ASSERT(!newFranchiseeId.empty(), "New Franchisee has generated UUID");
        std::cout << "    Created Franchisee with ID: " << newFranchiseeId << std::endl;

        // Update AppConfig with new franchisee ID
        bool configUpdated = client.setAppConfigValue("current_franchisee_id", newFranchiseeId);
        TEST_ASSERT(configUpdated, "AppConfig current_franchisee_id updated");

        // Verify AppConfig was updated
        std::string verifyId = client.getAppConfigValue("current_franchisee_id");
        TEST_ASSERT(verifyId == newFranchiseeId, "AppConfig current_franchisee_id matches new ID");
    }

    // --- POST New StoreLocation ---
    StoreLocationDTO newStore;
    newStore.id = "";  // Empty ID triggers POST with UUID generation
    newStore.franchiseeId = newFranchiseeId;  // Link to new franchisee
    newStore.storeName = "Test Store - POST Test";
    newStore.addressLine1 = "456 Test Avenue";
    newStore.city = "Test City";
    newStore.stateProvince = "TS";
    newStore.postalCode = "12345";
    newStore.latitude = 40.0;
    newStore.longitude = -105.0;
    newStore.defaultSearchRadiusMiles = 10.0;
    newStore.phone = "(555) 987-6543";
    newStore.isActive = true;
    newStore.isPrimary = true;

    auto storeResponse = client.saveStoreLocation(newStore);
    TEST_ASSERT(storeResponse.success, "StoreLocation POST successful");
    TEST_ASSERT(storeResponse.statusCode == 201 || storeResponse.statusCode == 200,
               "StoreLocation POST returned 201/200 status");

    std::string newStoreId;
    if (storeResponse.success) {
        auto created = StoreLocationDTO::fromJson(storeResponse.body);
        newStoreId = created.id;
        TEST_ASSERT(!newStoreId.empty(), "New StoreLocation has generated UUID");
        std::cout << "    Created StoreLocation with ID: " << newStoreId << std::endl;

        // Update AppConfig with new store ID
        bool configUpdated = client.setAppConfigValue("current_store_id", newStoreId);
        TEST_ASSERT(configUpdated, "AppConfig current_store_id updated");

        // Verify AppConfig was updated
        std::string verifyId = client.getAppConfigValue("current_store_id");
        TEST_ASSERT(verifyId == newStoreId, "AppConfig current_store_id matches new ID");
    }

    // --- Cleanup: Delete test records and restore original AppConfig ---
    std::cout << "\n    Cleaning up test records..." << std::endl;

    // Restore original AppConfig values
    if (!originalFranchiseeId.empty()) {
        client.setAppConfigValue("current_franchisee_id", originalFranchiseeId);
    }
    if (!originalStoreId.empty()) {
        client.setAppConfigValue("current_store_id", originalStoreId);
    }

    // Delete test store (if created)
    if (!newStoreId.empty()) {
        auto deleteResponse = client.deleteStoreLocation(newStoreId);
        TEST_ASSERT(deleteResponse.success, "Test StoreLocation deleted");
    }

    // Delete test franchisee (if created)
    if (!newFranchiseeId.empty()) {
        auto deleteResponse = client.deleteFranchisee(newFranchiseeId);
        TEST_ASSERT(deleteResponse.success, "Test Franchisee deleted");
    }

    std::cout << "    Restored original AppConfig values" << std::endl;
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main(int argc, char** argv) {
    std::cout << "============================================" << std::endl;
    std::cout << "ApiLogicServer Client Test Suite" << std::endl;
    std::cout << "============================================" << std::endl;

    // Create client (uses default config from app_config.json)
    ApiLogicServerClient client;

    // Run test cases
    test_retrieve_current_store_and_franchisee(client);
    test_patch_store_and_franchisee(client);
    test_post_new_store_and_franchisee(client);

    // Print summary
    std::cout << "\n============================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "  Passed: " << tests_passed << std::endl;
    std::cout << "  Failed: " << tests_failed << std::endl;
    std::cout << "  Total:  " << (tests_passed + tests_failed) << std::endl;

    if (tests_failed > 0) {
        std::cout << "\n  ✗ SOME TESTS FAILED" << std::endl;
        return 1;
    } else {
        std::cout << "\n  ✓ ALL TESTS PASSED" << std::endl;
        return 0;
    }
}

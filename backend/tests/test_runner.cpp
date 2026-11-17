#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include "transitx/algorithms/haversine.hpp"
#include "transitx/algorithms/eta.hpp"
#include "transitx/algorithms/route_matcher.hpp"
#include "transitx/utils/validation.hpp"
#include "jwt.hpp"
#include "bcrypt.hpp"

using namespace transitx;

void test_haversine() {
    std::cout << "[TEST] Running Haversine Distance Tests..." << std::endl;
    // Clock Tower Dehradun: 30.3255, 78.0437
    // ISBT Dehradun: 30.2885, 78.0084
    double d = algorithms::Haversine::calculate_distance_km(30.3255, 78.0437, 30.2885, 78.0084);
    std::cout << "  Distance Clock Tower to ISBT: " << d << " km" << std::endl;
    assert(d > 4.0 && d < 7.0);

    // Same point should be 0
    double zero_d = algorithms::Haversine::calculate_distance_km(30.3255, 78.0437, 30.3255, 78.0437);
    assert(zero_d < 0.001);

    // Initial bearing
    double bearing = algorithms::Haversine::calculate_bearing(30.3255, 78.0437, 30.2885, 78.0084);
    std::cout << "  Bearing Clock Tower to ISBT: " << bearing << " degrees" << std::endl;
    assert(bearing >= 0.0 && bearing <= 360.0);

    std::cout << "  -> PASS: Haversine tests" << std::endl;
}

void test_route_matching() {
    std::cout << "[TEST] Running Route Matcher Tests..." << std::endl;
    std::vector<algorithms::RoutePoint> stops = {
        {1, "Clock Tower", 30.3255, 78.0437, 1, 30},
        {2, "Prince Chowk", 30.3160, 78.0380, 2, 30},
        {3, "Saharanpur Chowk", 30.3080, 78.0280, 3, 30},
        {4, "ISBT Dehradun", 30.2885, 78.0084, 4, 30}
    };

    // Bus position near Prince Chowk
    auto match = algorithms::RouteMatcher::match_to_route(30.3162, 78.0382, stops);
    assert(match.is_matched == true);
    assert(match.current_segment_index >= 0);
    assert(match.next_stop_id == 2 || match.next_stop_id == 3);
    std::cout << "  Matched segment: " << match.current_segment_index << ", next stop ID: " << match.next_stop_id << std::endl;
    std::cout << "  Distance to route: " << match.distance_to_route_meters << " m" << std::endl;

    std::cout << "  -> PASS: Route Matcher tests" << std::endl;
}

void test_eta_calculation() {
    std::cout << "[TEST] Running ETA Engine Tests..." << std::endl;
    std::vector<algorithms::RoutePoint> stops = {
        {1, "Clock Tower", 30.3255, 78.0437, 1, 30},
        {2, "Prince Chowk", 30.3160, 78.0380, 2, 30},
        {3, "Saharanpur Chowk", 30.3080, 78.0280, 3, 30},
        {4, "ISBT Dehradun", 30.2885, 78.0084, 4, 30}
    };

    auto eta = algorithms::EtaCalculator::calculate_eta(
        30.3250, 78.0430, 25.0, "2026-11-17T10:00:00Z", stops, 4
    );

    std::cout << "  Remaining distance to stop 4: " << eta.remaining_distance_km << " km" << std::endl;
    std::cout << "  Estimated duration: " << eta.estimated_duration_minutes << " min" << std::endl;
    std::cout << "  Confidence: " << eta.confidence << std::endl;
    assert(eta.remaining_distance_km > 3.0);
    assert(eta.estimated_duration_minutes > 5.0);
    assert(eta.confidence > 0.5);

    std::cout << "  -> PASS: ETA Engine tests" << std::endl;
}

void test_validation() {
    std::cout << "[TEST] Running Validation Utility Tests..." << std::endl;
    assert(utils::Validation::is_valid_email("test@example.com") == true);
    assert(utils::Validation::is_valid_email("invalid-email") == false);
    assert(utils::Validation::is_valid_coordinates(30.3255, 78.0437) == true);
    assert(utils::Validation::is_valid_coordinates(95.0, 78.0437) == false);
    assert(utils::Validation::is_valid_coordinates(30.3255, 190.0) == false);
    assert(utils::Validation::is_valid_role("passenger") == true);
    assert(utils::Validation::is_valid_role("driver") == true);
    assert(utils::Validation::is_valid_role("admin") == true);
    assert(utils::Validation::is_valid_role("superhacker") == false);

    std::cout << "  -> PASS: Validation tests" << std::endl;
}

void test_jwt_and_auth() {
    std::cout << "[TEST] Running JWT & Password Hashing Tests..." << std::endl;
    std::string secret = "test_super_secret_key_12345";
    
    // Test JWT encode
    nlohmann::json payload;
    payload["sub"] = 42;
    payload["email"] = "driver1@demo.com";
    payload["role"] = "driver";

    std::string token = jwt::create(payload, secret, 3600);
    assert(!token.empty());
    std::cout << "  Generated JWT: " << token.substr(0, 30) << "..." << std::endl;

    // Test JWT decode & verify
    assert(jwt::verify(token, secret) == true);
    assert(jwt::verify(token, "wrong_secret") == false);

    auto claims = jwt::get_claims(token, secret);
    assert(claims["sub"].get<int>() == 42);
    assert(claims["email"].get<std::string>() == "driver1@demo.com");
    assert(claims["role"].get<std::string>() == "driver");

    // Test Bcrypt hash & verify
    std::string password = "password123";
    std::string hashed = bcrypt::generate_hash(password);
    assert(bcrypt::verify(password, hashed) == true);
    assert(bcrypt::verify("wrong_pass", hashed) == false);

    std::cout << "  -> PASS: JWT & Auth tests" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    TRANSITX C++ CORE TEST SUITE       " << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        test_haversine();
        test_route_matching();
        test_eta_calculation();
        test_validation();
        test_jwt_and_auth();

        std::cout << "========================================" << std::endl;
        std::cout << "   ALL C++ TESTS PASSED SUCCESSFULLY!   " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "TEST FAILED WITH EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}

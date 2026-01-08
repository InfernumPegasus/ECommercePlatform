# scripts/test_api.py
import requests
import json
import sys

BASE_URL = "http://localhost:1234"


def test_endpoint(name, method, endpoint, expected_status=200, data=None, **kwargs):
    """Тестирует endpoint и проверяет статус"""
    print(f"\n{'=' * 60}")
    print(f"Test: {name}")
    print(f"Endpoint: {method} {endpoint}")

    url = f"{BASE_URL}{endpoint}"

    try:
        if method.upper() == "GET":
            resp = requests.get(url, timeout=5, **kwargs)
        elif method.upper() == "POST":
            resp = requests.post(url, json=data, timeout=5, **kwargs)
        elif method.upper() == "PUT":
            resp = requests.put(url, json=data, timeout=5, **kwargs)
        elif method.upper() == "DELETE":
            resp = requests.delete(url, timeout=5, **kwargs)
        else:
            print(f"  ✗ Unknown method: {method}")
            return False

        print(f"  Status: {resp.status_code} (expected {expected_status})")
        print(f"  Response: {resp.text[:200]}...")

        if resp.status_code == expected_status:
            print(f"  ✓ PASS")
            return True
        else:
            print(f"  ✗ FAIL")
            return False

    except requests.exceptions.RequestException as e:
        print(f"  ✗ ERROR: {e}")
        return False


def run_tests():
    """Запуск всех тестов"""
    print("Starting API tests...")

    tests_passed = 0
    tests_failed = 0

    # Тесты для OrdersController
    tests = [
        ("List orders", "GET", "/orders", 200),
        ("Get order by ID", "GET", "/orders/123", 200),
        ("Get order with bool param", "GET", "/orders/123/name?random=true", 200),
        ("Update order name", "PUT", "/orders/123/name", 200, {"name": "Updated"}),
        ("Delete order", "DELETE", "/orders/123", 200),
        ("Remove all orders", "POST", "/orders/remove_all", 200),

        # Ошибочные сценарии
        ("Non-existent route", "GET", "/nonexistent", 404),
        ("Invalid order ID type", "GET", "/orders/abc", 404),  # abc не пройдет :int
        ("Missing required field", "PUT", "/orders/456/name", 400, {}),
        ("Extra path segments", "GET", "/orders/123/name/extra/more", 404),
    ]

    for test in tests:
        name, method, endpoint, expected = test[:4]
        data = test[4] if len(test) > 4 else None

        if test_endpoint(name, method, endpoint, expected, data):
            tests_passed += 1
        else:
            tests_failed += 1

    # Вывод статистики
    print(f"\n{'=' * 60}")
    print(f"TEST SUMMARY:")
    print(f"  Passed: {tests_passed}")
    print(f"  Failed: {tests_failed}")
    print(f"  Total:  {tests_passed + tests_failed}")

    return tests_failed == 0


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)

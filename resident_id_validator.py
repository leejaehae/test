#!/usr/bin/env python3
"""
Korean Resident Registration Number (주민등록번호) Validator
Format: YYMMDD-GNNNNNN
- YYMMDD: Birth date
- G: Gender digit (1-4 for Korean nationals, 5-8 for foreigners)
- NNNNNN: Serial number with last digit as checksum
"""

from datetime import datetime, date


def validate_resident_id(resident_id: str) -> dict:
    """
    Validate a Korean resident registration number.
    Returns a dict with validation results.
    """
    result = {
        "input": resident_id,
        "valid": False,
        "errors": [],
        "birth_date": None,
        "gender": None,
        "age": None
    }

    # Remove hyphen if present
    cleaned = resident_id.replace("-", "").replace(" ", "")

    # Check length
    if len(cleaned) != 13:
        result["errors"].append(f"길이 오류: {len(cleaned)}자 (13자 필요)")
        return result

    # Check if all digits
    if not cleaned.isdigit():
        result["errors"].append("숫자가 아닌 문자 포함")
        return result

    # Extract parts
    birth_part = cleaned[:6]
    gender_digit = int(cleaned[6])
    serial_part = cleaned[7:12]
    check_digit = int(cleaned[12])

    # Validate gender digit
    if gender_digit not in range(1, 9) and gender_digit != 0:
        result["errors"].append(f"성별 코드 오류: {gender_digit}")
        return result

    # Determine century based on gender digit
    if gender_digit in [1, 2, 5, 6]:
        century = 1900
    elif gender_digit in [3, 4, 7, 8]:
        century = 2000
    elif gender_digit in [9, 0]:
        century = 1800
    else:
        result["errors"].append(f"성별 코드 오류: {gender_digit}")
        return result

    # Parse birth date
    try:
        year = century + int(birth_part[:2])
        month = int(birth_part[2:4])
        day = int(birth_part[4:6])
        birth_date = date(year, month, day)
        result["birth_date"] = birth_date.strftime("%Y-%m-%d")

        # Check if date is in the future
        today = date.today()
        if birth_date > today:
            result["errors"].append(f"미래 날짜: {birth_date}")
        else:
            # Calculate age
            age = today.year - birth_date.year
            if (today.month, today.day) < (birth_date.month, birth_date.day):
                age -= 1
            result["age"] = age

    except ValueError as e:
        result["errors"].append(f"날짜 오류: {birth_part} ({str(e)})")
        return result

    # Determine gender
    if gender_digit in [1, 3, 5, 7, 9]:
        result["gender"] = "남성"
    else:
        result["gender"] = "여성"

    # Validate checksum
    weights = [2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4, 5]
    total = sum(int(cleaned[i]) * weights[i] for i in range(12))
    expected_check = (11 - (total % 11)) % 10

    if check_digit != expected_check:
        result["errors"].append(f"체크섬 오류: 입력값={check_digit}, 기대값={expected_check}")

    # Set valid flag
    if not result["errors"]:
        result["valid"] = True

    return result


def run_tests():
    """Run validation tests on the provided resident IDs."""
    test_ids = [
        "910602-3023117",
        "910702-4023118",
        "910802-1023119",
        "910902-2023110",
        "910203-3023112",
        "410204-4023112",
        "510202-3023113",
        "610302-4023114",
        "110402-1023115",
        "210502-2023116",
        "310602-3023117",
        "610702-4023118",
        "880802-1023119",
        "920902-2023110",
        "930203-3023112",
        "940204-4023112",
    ]

    print("=" * 70)
    print("주민등록번호 유효성 검사 결과")
    print("=" * 70)

    valid_count = 0
    invalid_count = 0

    for i, test_id in enumerate(test_ids, 1):
        result = validate_resident_id(test_id)
        status = "✓ 유효" if result["valid"] else "✗ 무효"

        print(f"\n[{i:02d}] {test_id}")
        print(f"    상태: {status}")

        if result["birth_date"]:
            print(f"    생년월일: {result['birth_date']}")
        if result["gender"]:
            print(f"    성별: {result['gender']}")
        if result["age"] is not None:
            print(f"    나이: {result['age']}세")

        if result["errors"]:
            for error in result["errors"]:
                print(f"    오류: {error}")

        if result["valid"]:
            valid_count += 1
        else:
            invalid_count += 1

    print("\n" + "=" * 70)
    print(f"검사 결과 요약: 총 {len(test_ids)}건 | 유효: {valid_count}건 | 무효: {invalid_count}건")
    print("=" * 70)

    return valid_count, invalid_count


if __name__ == "__main__":
    run_tests()

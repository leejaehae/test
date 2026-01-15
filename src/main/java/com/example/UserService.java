package com.example;

import java.util.*;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * 사용자 서비스 클래스
 */
public class UserService {

    private static final Pattern EMAIL_PATTERN =
        Pattern.compile("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$");

    private final Map<Long, User> userRepository = new HashMap<>();
    private long nextId = 1L;

    public User createUser(String name, String email, int age) {
        validateName(name);
        validateEmail(email);
        validateAge(age);

        if (isEmailTaken(email)) {
            throw new IllegalStateException("이미 사용 중인 이메일입니다: " + email);
        }

        User user = new User(nextId++, name, email, age);
        userRepository.put(user.getId(), user);
        return user;
    }

    public Optional<User> findById(Long id) {
        return Optional.ofNullable(userRepository.get(id));
    }

    public Optional<User> findByEmail(String email) {
        return userRepository.values().stream()
            .filter(user -> user.getEmail().equals(email))
            .findFirst();
    }

    public List<User> findAll() {
        return new ArrayList<>(userRepository.values());
    }

    public List<User> findByAgeRange(int minAge, int maxAge) {
        return userRepository.values().stream()
            .filter(user -> user.getAge() >= minAge && user.getAge() <= maxAge)
            .collect(Collectors.toList());
    }

    public User updateUser(Long id, String name, String email, int age) {
        User user = userRepository.get(id);
        if (user == null) {
            throw new IllegalArgumentException("사용자를 찾을 수 없습니다: " + id);
        }

        validateName(name);
        validateEmail(email);
        validateAge(age);

        if (!user.getEmail().equals(email) && isEmailTaken(email)) {
            throw new IllegalStateException("이미 사용 중인 이메일입니다: " + email);
        }

        user.setName(name);
        user.setEmail(email);
        user.setAge(age);
        return user;
    }

    public boolean deleteUser(Long id) {
        return userRepository.remove(id) != null;
    }

    public int countUsers() {
        return userRepository.size();
    }

    public double getAverageAge() {
        if (userRepository.isEmpty()) {
            return 0.0;
        }
        return userRepository.values().stream()
            .mapToInt(User::getAge)
            .average()
            .orElse(0.0);
    }

    private void validateName(String name) {
        if (name == null || name.trim().isEmpty()) {
            throw new IllegalArgumentException("이름은 필수입니다.");
        }
        if (name.length() > 50) {
            throw new IllegalArgumentException("이름은 50자를 초과할 수 없습니다.");
        }
    }

    private void validateEmail(String email) {
        if (email == null || email.trim().isEmpty()) {
            throw new IllegalArgumentException("이메일은 필수입니다.");
        }
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            throw new IllegalArgumentException("유효하지 않은 이메일 형식입니다.");
        }
    }

    private void validateAge(int age) {
        if (age < 0 || age > 150) {
            throw new IllegalArgumentException("나이는 0에서 150 사이여야 합니다.");
        }
    }

    private boolean isEmailTaken(String email) {
        return userRepository.values().stream()
            .anyMatch(user -> user.getEmail().equals(email));
    }
}

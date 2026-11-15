package com.main.model;

public record Contact(String name, String number) {
    @Override
    public String toString() {
        return name + " | " + number;
    }
}
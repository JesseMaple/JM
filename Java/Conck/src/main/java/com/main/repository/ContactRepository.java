package com.main.repository;

import com.main.model.Contact;
import java.util.List;

public interface ContactRepository {
    List<Contact> findAll();
    void saveAll(List<Contact> contacts);
}
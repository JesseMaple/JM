package com.main.service;

import com.main.model.Contact;
import com.main.repository.ContactRepository;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

public class ContactService {
    private final List<Contact> contacts;
    private final ContactRepository repo;

    public ContactService(ContactRepository repo) {
        this.repo = repo;
        this.contacts = new ArrayList<>(repo.findAll());
    }

    public List<Contact> getContacts() { return List.copyOf(contacts); }

    public List<Contact> search(String keyword) {
        String k = keyword.toLowerCase();
        return contacts.stream()
                .filter(c -> c.name().toLowerCase().contains(k) || c.number().toLowerCase().contains(k))
                .toList();
    }

    public List<Contact> rearrangeByName() {
        return contacts.stream()
                .sorted(Comparator.comparing(Contact::name, String.CASE_INSENSITIVE_ORDER).thenComparing(Contact::number))
                .toList();
    }

    public List<Contact> rearrangeByNumber() {
        return contacts.stream()
                .sorted(Comparator.comparing(Contact::number).thenComparing(Contact::name))
                .toList();
    }

    public boolean add(String name, String number) {
        Contact c = new Contact(name, number);
        if (contacts.contains(c)) return false;
        contacts.add(c);
        repo.saveAll(contacts);
        return true;
    }

    public boolean remove(Contact c) {
        if (!contacts.remove(c)) return false;
        repo.saveAll(contacts);
        return true;
    }

    public boolean update(Contact oldC, String newName, String newNumber) {
        int i = contacts.indexOf(oldC);
        if (i < 0) return false;
        contacts.set(i, new Contact(newName, newNumber));
        repo.saveAll(contacts);
        return true;
    }
}
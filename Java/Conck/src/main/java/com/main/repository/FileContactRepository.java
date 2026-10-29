package com.main.repository;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import com.main.model.Contact;

import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.lang.reflect.Type;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;

public class FileContactRepository implements ContactRepository {
    private static final Gson GSON = new GsonBuilder().setPrettyPrinting().create();
    private static final Type LIST_TYPE = new TypeToken<List<Contact>>() {}.getType();

    private final Path path;

    public FileContactRepository(String filepath) {
        this.path = Path.of(filepath);
    }

    @Override
    public List<Contact> findAll() {
        if (Files.notExists(path)) return new ArrayList<>();
        try (Reader r = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            List<Contact> list = GSON.fromJson(r, LIST_TYPE);
            return list == null ? new ArrayList<>() : list;
        } catch (IOException e) {
            System.err.println("Failed to load contacts: " + e.getMessage());
            return new ArrayList<>();
        }
    }

    @Override
    public void saveAll(List<Contact> contacts) {
        Path tmp = path.resolveSibling(path.getFileName() + ".tmp");
        try {
            Path parent = path.toAbsolutePath().getParent();
            if (parent != null) Files.createDirectories(parent);
            try (Writer w = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8)) {
                GSON.toJson(contacts, w);
            }
            try {
                Files.move(tmp, path, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException ex) {
                Files.move(tmp, path, StandardCopyOption.REPLACE_EXISTING);
            }
        } catch (IOException e) {
            System.err.println("Failed to save contacts: " + e.getMessage());
        }
    }
}
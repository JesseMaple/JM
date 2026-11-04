package com.main;

import com.main.model.Contact;
import com.main.repository.FileContactRepository;
import com.main.service.ContactService;
import javafx.application.Application;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

public class App extends Application {

    private final ContactService service = new ContactService(new FileContactRepository("contacts.json"));
    private final ObservableList<Contact> tableData = FXCollections.observableArrayList();
    private TableView<Contact> table;

    @Override
    public void start(Stage stage) {
        stage.setTitle("Conck - Contacts");

        TextField searchField = new TextField();
        searchField.setPromptText("Search name or number...");
        searchField.textProperty().addListener((obs, old, val) -> refreshTable(val));

        Button addBtn = new Button("Add");
        Button editBtn = new Button("Edit");
        Button deleteBtn = new Button("Delete");
        Button sortByNameBtn = new Button("Sort by Name");
        Button sortByNumberBtn = new Button("Sort by Number");
        Button refreshBtn = new Button("Refresh");

        HBox toolBar = new HBox(8, addBtn, editBtn, deleteBtn, new Separator(), sortByNameBtn, sortByNumberBtn, refreshBtn);
        toolBar.setPadding(new Insets(8));

        table = new TableView<>(tableData);
        table.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY_FLEX_LAST_COLUMN);

        TableColumn<Contact, String> nameCol = new TableColumn<>("Name");
        // 使用 lambda 获取 record 属性
        nameCol.setCellValueFactory(data -> new SimpleStringProperty(data.getValue().name()));

        TableColumn<Contact, String> numberCol = new TableColumn<>("Number");
        numberCol.setCellValueFactory(data -> new SimpleStringProperty(data.getValue().number()));

        table.getColumns().addAll(nameCol, numberCol);

        addBtn.setOnAction(e -> showAddDialog());
        editBtn.setOnAction(e -> {
            Contact selected = table.getSelectionModel().getSelectedItem();
            if (selected != null) showEditDialog(selected);
        });
        deleteBtn.setOnAction(e -> {
            Contact selected = table.getSelectionModel().getSelectedItem();
            if (selected != null) {
                service.remove(selected);
                refreshTable(searchField.getText());
            }
        });
        sortByNameBtn.setOnAction(e -> tableData.setAll(service.rearrangeByName()));
        sortByNumberBtn.setOnAction(e -> tableData.setAll(service.rearrangeByNumber()));
        refreshBtn.setOnAction(e -> refreshTable(searchField.getText()));

        refreshTable("");

        BorderPane root = new BorderPane();
        root.setTop(new HBox(10, searchField, toolBar));
        root.setCenter(table);
        root.setPadding(new Insets(10));

        Scene scene = new Scene(root, 650, 450);
        stage.setScene(scene);
        stage.show();
    }

    private void refreshTable(String keyword) {
        if (keyword == null || keyword.isBlank()) {
            tableData.setAll(service.getContacts());
        } else {
            tableData.setAll(service.search(keyword));
        }
    }

    private void showAddDialog() {
        Dialog<Contact> dialog = new Dialog<>();
        dialog.setTitle("Add Contact");

        TextField nameField = new TextField();
        TextField numberField = new TextField();
        dialog.getDialogPane().setContent(new VBox(8, new Label("Name:"), nameField, new Label("Number:"), numberField));

        ButtonType okType = new ButtonType("Add", ButtonBar.ButtonData.OK_DONE);
        dialog.getDialogPane().getButtonTypes().addAll(okType, ButtonType.CANCEL);

        dialog.setResultConverter(btn -> btn == okType ? new Contact(nameField.getText().trim(), numberField.getText().trim()) : null);

        dialog.showAndWait().ifPresent(c -> {
            if (!service.add(c.name(), c.number())) {
                new Alert(Alert.AlertType.WARNING, "Contact already exists.").showAndWait();
            }
            refreshTable("");
        });
    }

    private void showEditDialog(Contact old) {
        Dialog<Contact> dialog = new Dialog<>();
        dialog.setTitle("Edit Contact");

        TextField nameField = new TextField(old.name());
        TextField numberField = new TextField(old.number());
        dialog.getDialogPane().setContent(new VBox(8, new Label("Name:"), nameField, new Label("Number:"), numberField));

        ButtonType okType = new ButtonType("Save", ButtonBar.ButtonData.OK_DONE);
        dialog.getDialogPane().getButtonTypes().addAll(okType, ButtonType.CANCEL);

        dialog.setResultConverter(btn -> btn == okType ? new Contact(nameField.getText().trim(), numberField.getText().trim()) : null);

        dialog.showAndWait().ifPresent(updated -> {
            service.update(old, updated.name(), updated.number());
            refreshTable("");
        });
    }

    public static void main(String[] args) {
        launch();
    }
}
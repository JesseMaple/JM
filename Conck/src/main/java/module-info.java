module com.main {
    requires javafx.controls;
    requires com.google.gson;

    exports com.main;
    opens com.main.model to com.google.gson;
}
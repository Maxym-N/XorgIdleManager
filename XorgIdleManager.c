/*
 * XorgIdleManager: a utility designed to manage system sleep or shutdown based on inactivity in X11 environments.
 *
 * Version: 1.5
 * 
 * Copyright (C) 2025 Maksym Nazar.
 * Created with the assistance of Perplexity, ChatGPT and Claude.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_PATH_LENGTH 1024
#define MAX_ERROR_LENGTH 1024

static const char icon_base64[] =
    "iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAACXBIWXMAAA3XAAAN1wFCKJt4AAAC61BMVEVHcEwojN1/7vpDpOVc7Pxr8f6U+Poo1fc/3PdS5fkix/WW+/0etvGJ+f2T+vyV+/+R/P6T+/0gddMamucbc9Adg9yU+/0dddMddtMYlecYkuEdhuAXpemT+/2V+P0cctIYquqM+v539P1BSHonHv4lG0QnKVMlHkWO/P4oK1j+/v4cctAmH0omJ1AmHEIXzvcoHUWN+f4lGkcHmOoFn+qU/P3W4/5mMNWlmv0nIFUgFkElHE77/PwoL16rpP2onv0iHUYqKFMSyPd0+vwKvvYhGEURXcwRY88Wre2E+vx+TevQ3f3f6v6CT+sGpO3Y2eAYf9ggG0n6SowfcMsbeNGu/v5nNtpmrL4fS5FmN+AbndD39/kfUZWGUewQatEqJmgnIl0YuO74Q5NpMtvv7/OSkagyLXUVi94VouqFTPIJkelBXXv+XInsMqD8U4uena8cOXQxLVfo8f3I2v4dMGmPT/EfIVYX1fdIQmciJFmJS/EUmeLzNZlvcY3+gnud/v3i4+q8vclAO2Cwqf4tJUwOUccVwfN+Ue7JytOm/v5HLpF+PNLBufb+coBXVXcbyfGfjvRq4/QaX6Pv9/599v0fVMMhuuJOLKv+ZYY2PGp/e5ZgorVoZ4SvrruIiJ5TTGkkrdhBIXY3MlsXSZ0TbKtzOMcbQ4UeHVAgK2ATU9J/TfdBQ3clPmm1tsQoQLWXevKmlPw4MITp6u78kXpiX34QddX0z92snMtkLNo7weXD0PV0xNNZxd9WhaMgfOROT3A7IkoqRXq2xvrvWqe4qNmM7fUeYrUSZd0fL3ylK4JYS3azo9S1sPan9/d0cpWYUe0Tg8HaTnzmdnagqb59aoj34Oj0v9zBst6B3ORttcb7fZKDnLMeWoWXlebb2fiOitz5noxJL0xwOujvhL6Ed9GrMHuhm/BIbo2MKXX6xLmywNpsYLLV2evS7/iT1vOdVfprM86dlsbxqMVaKr67sNNgnNY8irySNYKdUfRzA8gWAAAAI3RSTlMAG/kJ/vv8/v3++i760G9S4ZN8mO/Cs925Z+pRy4C4o+zCrdI1TwQAAApnSURBVFjDbZd5WBNnHseD9aKuR9Xe7bb2eUjKJANDOCTJAAkQSKQSIqwKQQVJIICaRGlIjGgSI6aKQSGIIIdGRORQYal4IUXEtmrV1lu7brfa+9i22z3+3N/7ziSAT7/JHJnM5/P+3ndmkhkOZywzZr0wZ2p0dPTbTPzLiYmYs+D1GZw/yuwFwG4Z42HfcZlomTnrmafxGTOjce6dvb8Msh/eeMJZFsj9s/cY15zZE/lZqPSp979piAuNC6TB01QBafJ0NcSN2/rNslGkeG48/xzC9zeExoWyiRtqcdhpwh+tW+dhv5HB1D04Cp16Yawbz0VERN8rZffghoZadHYEGRqbPK2tnqZGnaOPIOwGC3yJAo6GszBCLwTqB/5sdyjzdWjoUA9N2HWtYZCQMCEVEhICS0uLiqB7hrhMoMRlMLKvs+M3NSLibFwcYw91NtOaZqAxJxRAhBD0qbWHppudAcWyCGUEczz/HBFxz9++rKmPaHaGhFDQsMsjFwokYHAKQoRCCgqyOIi+JrQbiQxnI5Qz8fGPjogoxTzJNRlolUcqhQYpodSjEqsFQjXp1vRZoAiKogSSfjttkLE1dI8qlbOZAu4zzZOmHqLZ5ex3gkIo0dF0n0mtlnhUR24aUCUU5Sv1wS4Ok4wZykElKiEIRqALC0iTg9DxWmpra69JBRKJqVE1JJFIjjRoW1QtEolIwiv95C+fDIoMhNuHDaa4Ua93Bmc2jCDTAeBbeIba2vW1tRVSiUjkuwYzkdncpGo2wYoUeMhDiQ5qwCVw93u9s+AcUA5yES8zEAa5DniUCqkoLw/eeR5PntVshRXEv8cYDESzDIX7jdf7FmeBUlmKeLKCcMh3VLGC9RXyvFWQLreqAehViH8PAoLRNnUP0SiTkaSs2+udyZmj9MZBl0inVuXqXbTiDMDvoDCGa25VFxKpGR4Mo+npuTKVZogEAXfPlCmcSd5JsEaSDrpVtKh4xYoqhgeDdFVhYWFXF8yAn7plyxbM69PT042tGjdUQJLnpkwGwR44Lch+ollcDzwYMB6DakAGFCnmkWE0PSo9PeqGXEdUkCTXdW4yCJL2wHhyVbRFfXjFCtYQ805MTAwy9PYW9hZC+9GMYBLwURCpq89uAgMWTNkDh6SCuPNfwY7tWLC4CuEo17BBXap8G/1YbdkyKSoKG9rk8kaiEQuCOVNAIJO5iZG13wqRYTGkihXENMoLe4FXol/I6Og9kZFRSNEGlxaU4AJBMCPgWog7a9eu/Va+YzGTMYNaVOpVKpGC5aPSES9U64h+GEQkmAyDqCMeg2Dtx8IxQxpKTMzDQa83CRuAj2R5uEglTrqZJE9Pn86ZPDmYhAtuLc7Hkh2LIGC4yhrSkpKSlMgQ4KVCCYrIrZVhQXDwdNJH39m0aRNbw/Zxhpi0DYj3sjwYgBcdgUhELcQQeYAV9BOPQbBpZATXgA2LFl9FFWzY4E1KgjfikaBGLBSJuKFgMHuIFvLAtOmc6dNPky3EyGoQ3EF1sAbIVcCxICkpOLIEdwDzMqfW0XDEHKppBsE0JBA0E6tXrx55fPv24xHGsB07rvoFwSUlJaiENnEIusJFTUSfxWxWqQSsQOrWgOAO+gO4AytfIQPwx09cYATByekgKIlskwsRb7Va3YTKanVoAwLV7SVLVi9Zd/LkyGqUr+TIcDw29kQVEgQnJyeml+D24ecFcK7VQRD91pu0CQumnZbaT65bAvnttyVIBAY4FsDHngAD4sGgiMT991nMVrM5TksQOrOOduaET+NMm7ZX2ndy3Tqs8Odb9Q+xsYwB85Ab4pC8PGuTvcJqbnAQ9HoQEJYJgrEs+ZTqupqADbHH9ayg/HOxKM/alfZIA81r0tIqrFgQznkWBPaTa8aybt2nHzTEb90KdELC7lNH9YnJyfnJ5eVgMHd0fH/mQtqjMxcunInrMBDOnGpGoLq9b82afWse7NsHyzXAf711KzIk7F546uhRfWVlcnh5OBjkeWVdPwINfGlHBwxiTnU159nqvVKHBtB9D07u2gWLTz/wfR0PFWyNjz2+FAQpKfpKaD88PLx8WNxR1tUDJ+j3pR1lZQ6NAAvK90oN9INdEDQD3vS3eCzYuvtUQUHB0dTUVAXCQVDeLS6DDA+jufFHFQ8LwvdKG+kvgN28efOuXZ8xPFJ8vfBUdkH2hx9+eHSbItxvkJcZy4zG0uEy47Cmh5dTWYm7MER/t5nJGB+/e+HChae+BME2iIIVVCMDsCqjcZBu4eXkY4HApb09nk9g+V+QAdpnDYlIUd2tNra333xobL9JtwYEgh76i79CPvuH7O8J8QkJCUz7kIJ/YTxl21FUQyI4kGF5e3v7crudB4J8zp8qQdBEf+fnE5AhfvdSlpdIDqUwQTVAwqvL1Mtzl+cO0oZxAlmfFkpg+ASWhyNQUC85WKg+lJqKBFBDot+QW1PzPdHKo7Ag/wDcxejo775AfCy84PxD7SNeeHDnsVXyQyuxIlWBBYnVezuOtP1H4w4BgcIv8Gk1t8ToAkICzC9l+J3Hjq0STzRUVueIu88Q/VigYAWCFrpF+iSWOf8xv3Rpvfrg4Z07654yVCZWVubLW2k3jxcQ8HgwCna76YfYE2AI8ELE/+88awBFaupKZKisvBHmplulYwIpMjRpDJInJ2Jx+0sD/PV/ns8Aw0fIgBQr84HPIW+hQ8DjCZHgzXyFgIcMhh/7RU8SdmcXAJ2dXR9y8PDhndeR4P26Y3UfyZEBHGA4wLXcVblQD6Q2vZ7zmiK/AwtMv/9uET3JzkZ4Nmr/8M53r19/93xGVkZdXR1TA0pOh+/KXScgYVSoXv8mZ55C8TniBZTzyhWLpD4bpV6OeZTzGRDG8D5k5SW1b+BiKw9nWG97jfOyXnEDcOSwgEFdz/C//IL5jRvPFyFDit9QI/QN3L1MUYin2my2eZwgvV5vFTAbnAMXL4sP7ugVsvzGjUiAk1qX8pHYeMgotsA+YWG4ALLIZnuVw3lNr69hBDzK9+vFRpdY3ruw4EsG31hcwqSIn5LSLhZTty4OOFmeWm6zvQIPHS/bbLYOpk88ytV4ceDyjuxTXwJeXIwF6G8xEgwZqSntlwcu6kyUv4CsoqJ5cLP9zCs22w0Ba+AJLw/cvXu8s7OzuLh4MUyd+K4kChS2on//emXAQ6H7ftxYTUbRK0Hofv9Vm62oBhtC0Mx1za7RVrGO4k50XwjRn/55vUZ1yxWGA7f+VDuM7VzmkWUeDFKu1F8DRcF9Ok1rq04cX1Hc2alXKPJPn/v5J42mT+ekwvwCwTA/K+N59pnpmefBUCPhjSkojwEcYHn0aMNP6zUQe3O/iwqjAnwun5/1YhAnYIByMo0BQxgPjoqvSdej0mo0WpXDcMviRxnceInP54/xYHgpIyMr61J7h4DHnCNMHRCXgHnaodB2Cm0UxLVfyszk818KmvDo+vL8LJTMSzW5OMufDrM5twbgTH5m5vy5Tz88B72BBfDK4mfx0RuFXUAy8ZSJw58/N+gPHt+D5r40P0Bl8icGPmcybWe++Mar457d/w+H3IXxpooBiQAAAABJRU5ErkJggg==";

static void set_window_icon(GtkWidget *window) {
    g_print("Attempting to set window icon...\n");
    GdkPixbuf *icon = NULL;
    GError *error = NULL;
    gsize data_len;

    guchar* data = g_base64_decode(icon_base64, &data_len);
    if (!data) {
        g_warning("Failed to decode Base64 data\n");
        return;
    }

    GInputStream *stream = g_memory_input_stream_new_from_data(data, data_len, g_free);
    if (!stream) {
        g_warning("Failed to create input stream\n");
        g_free(data);
        return;
    }
    g_print("Input stream created successfully\n");

    icon = gdk_pixbuf_new_from_stream(stream, NULL, &error);
    g_object_unref(stream);

    if (error) {
        g_warning("Error loading icon: %s", error->message);
        g_error_free(error);
        return;
    }

    if (icon) {
        int width = gdk_pixbuf_get_width(icon);
        int height = gdk_pixbuf_get_height(icon);
        g_print("Original icon size: %dx%d\n", width, height);

        GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 
            width,
            height,
            GDK_INTERP_BILINEAR);

        if (scaled_icon) {
            gtk_window_set_icon(GTK_WINDOW(window), scaled_icon);
            g_object_unref(scaled_icon);
            g_print("Icon set successfully at %dx%d\n", width, height);
        } else {
            g_warning("Failed to process icon\n");
        }
        g_object_unref(icon);
    } else {
        g_warning("Failed to create icon from data\n");
    }
}

const int DEFAULT_INACTIVITY_TIMEOUT_SLEEP = 3600000;
const int DEFAULT_INACTIVITY_TIMEOUT_SHUTDOWN = 64800000;
const int DEFAULT_INACTIVITY_TIMEOUT_NO_GUI = 10800000;
const int DEFAULT_LONG_RUNNING_APPS_TIMEOUT_SLEEP = 10800000;
const int DEFAULT_LONG_RUNNING_APPS_TIMEOUT_SHUTDOWN = 64800000;
const int DEFAULT_LONG_RUNNING_APPS_TIMEOUT_NO_GUI = 10800000;
const int DEFAULT_CHECK_INTERVAL = 3600;
const int DEFAULT_CPU_THRESHOLD = 50;
const int DEFAULT_IO_THRESHOLD = 5000;
const int DEFAULT_NET_THRESHOLD = 250;
const int DEFAULT_CPU_NET_CHECK_INTERVAL = 5;
const int DEFAULT_IO_CHECK_INTERVAL = 1;
const int DEFAULT_BOOT_GRACE_PERIOD = 20;
const int DEFAULT_ACTION_TYPE_SLEEP = 0;
const int DEFAULT_ACTION_TYPE_SHUTDOWN = 1;
const char *DEFAULT_CUSTOM_LONG_RUNNING_APPS = "smplayer|mplayer|vlc";
const char *DEFAULT_SLEEP_COMMAND = "sync; pm-suspend";
const char *DEFAULT_SHUTDOWN_COMMAND = "sync; shutdown -h now";
const char *DEFAULT_CUSTOM_NO_GUI_COMMAND = "sync; shutdown -h now";

int inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT_SLEEP;
int long_running_apps_timeout = DEFAULT_LONG_RUNNING_APPS_TIMEOUT_SLEEP;
int check_interval = DEFAULT_CHECK_INTERVAL;
int cpu_threshold = DEFAULT_CPU_THRESHOLD;
int io_threshold = DEFAULT_IO_THRESHOLD;
int net_threshold = DEFAULT_NET_THRESHOLD;
int cpu_net_check_interval = DEFAULT_CPU_NET_CHECK_INTERVAL;
int io_check_interval = DEFAULT_IO_CHECK_INTERVAL;
int boot_grace_period = DEFAULT_BOOT_GRACE_PERIOD;
int action_type = DEFAULT_ACTION_TYPE_SLEEP;
char custom_long_running_apps[MAX_COMMAND_LENGTH];
char sleep_command[MAX_COMMAND_LENGTH];
char shutdown_command[MAX_COMMAND_LENGTH];
char custom_no_gui_command[MAX_COMMAND_LENGTH];
int dpms_enabled = 1;
int dpms_standby = 600;
int dpms_suspend = 600;
int dpms_off = 600;

GtkWidget *inactivity_timeout_entry;
GtkWidget *long_running_apps_timeout_entry;
GtkWidget *custom_long_running_apps_entry;
GtkWidget *check_interval_entry;
GtkWidget *cpu_threshold_entry;
GtkWidget *io_threshold_entry;
GtkWidget *net_threshold_entry;
GtkWidget *cpu_net_check_interval_entry;
GtkWidget *io_check_interval_entry;
GtkWidget *boot_grace_period_entry;
GtkWidget *action_combo_box;
GtkWidget *sleep_command_entry;
GtkWidget *shutdown_command_entry;
GtkWidget *custom_no_gui_command_entry;
GtkWidget *dpms_enabled_check;
GtkWidget *dpms_standby_entry;
GtkWidget *dpms_suspend_entry;
GtkWidget *dpms_off_entry;

gboolean restart_daemon_async(gpointer user_data);

void show_error(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_info(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int validate_integer_input(const char *input, int min, int max) {
    char *endptr;
    long value = strtol(input, &endptr, 10);
    
    if (*input == '\0' || *endptr != '\0' || errno == ERANGE) {
        return 0;
    }
    
    return (value >= min && value <= max);
}

void ensure_config_dir_exists(void) {
    char config_dir[MAX_PATH_LENGTH];
    snprintf(config_dir, sizeof(config_dir), "%s/.XorgIdleManager", getenv("HOME"));
    
    struct stat st = {0};
    if (stat(config_dir, &st) == -1) {
        if (mkdir(config_dir, 0755) == -1) {
            char error_message[MAX_ERROR_LENGTH];
            snprintf(error_message, sizeof(error_message), "Failed to create config directory: %s", strerror(errno));
            show_error(error_message);
        }
    }
}

void execute_command(const char* command, char* const argv[]) {
    pid_t pid = fork();
    if (pid == -1) {
        show_error("Failed to fork process.");
        return;
    }

    if (pid == 0) {
        execvp(command, argv);
        _exit(EXIT_FAILURE);
    } else {
        waitpid(pid, NULL, 0);
    }
}

void get_dpms_settings() {
    pid_t pid = fork();
    if (pid == -1) {
        show_error("Failed to fork process for DPMS settings.");
        return;
    }

    if (pid == 0) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            _exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();
        if (child_pid == -1) {
            _exit(EXIT_FAILURE);
        }

        if (child_pid == 0) { 
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            execlp("xset", "xset", "q", NULL);
            _exit(EXIT_FAILURE); 
        } 
        
        close(pipe_fd[1]);
        FILE* stream_fp = fdopen(pipe_fd[0], "r");
        if (stream_fp == NULL) {
            _exit(EXIT_FAILURE);
        }

        char line[256];
        while (fgets(line, sizeof(line), stream_fp) != NULL) {
            if (strstr(line, "DPMS is Enabled")) {
                dpms_enabled = 1;
            } else if (strstr(line, "DPMS is Disabled")) {
                dpms_enabled = 0;
            } else if (strstr(line, "Standby:")) {
                sscanf(line, " Standby: %d Suspend: %d Off: %d", 
                       &dpms_standby, &dpms_suspend, &dpms_off);
            }
        }

        fclose(stream_fp);
        waitpid(child_pid, NULL, 0);
        _exit(EXIT_SUCCESS);
    } 
    
    int status;
    waitpid(pid, &status, 0);
}

void read_config(void) {
    ensure_config_dir_exists();
    
    char config_path[MAX_PATH_LENGTH];
    snprintf(config_path, sizeof(config_path), "%s/.XorgIdleManager/config.ini", getenv("HOME"));
    FILE *fp = fopen(config_path, "r");
    if (fp == NULL) {
        strncpy(custom_long_running_apps, DEFAULT_CUSTOM_LONG_RUNNING_APPS, sizeof(custom_long_running_apps) - 1);
        strncpy(sleep_command, DEFAULT_SLEEP_COMMAND, sizeof(sleep_command) - 1);
        strncpy(shutdown_command, DEFAULT_SHUTDOWN_COMMAND, sizeof(shutdown_command) - 1);
        strncpy(custom_no_gui_command, DEFAULT_CUSTOM_NO_GUI_COMMAND, sizeof(custom_no_gui_command) - 1);
        return;
    }
    
    char line[MAX_COMMAND_LENGTH * 2];
    while (fgets(line, sizeof(line), fp) != NULL) {
        char key[64], value[MAX_COMMAND_LENGTH * 2];
        if (sscanf(line, "%63[^=]=%[^\n]", key, value) == 2) {
            if (strcmp(key, "inactivity_timeout") == 0) {
                inactivity_timeout = atoi(value);
            } else if (strcmp(key, "long_running_apps_timeout") == 0) {
                long_running_apps_timeout = atoi(value);
            } else if (strcmp(key, "custom_long_running_apps") == 0) {
                strncpy(custom_long_running_apps, value, sizeof(custom_long_running_apps) - 1);
            } else if (strcmp(key, "check_interval") == 0) {
                check_interval = atoi(value);
            } else if (strcmp(key, "cpu_threshold") == 0) {
                cpu_threshold = atoi(value);
            } else if (strcmp(key, "io_threshold") == 0) {
                io_threshold = atoi(value);
            } else if (strcmp(key, "net_threshold") == 0) {
                net_threshold = atoi(value);
            } else if (strcmp(key, "cpu_net_check_interval") == 0) {
                cpu_net_check_interval = atoi(value);
            } else if (strcmp(key, "io_check_interval") == 0) {
                io_check_interval = atoi(value);
            } else if (strcmp(key, "boot_grace_period") == 0) {
                boot_grace_period = atoi(value);
            } else if (strcmp(key, "action_type") == 0) {
                action_type = atoi(value);
            } else if (strcmp(key, "sleep_command") == 0) {
                strncpy(sleep_command, value + 1, sizeof(sleep_command) - 1);
                sleep_command[strlen(sleep_command) - 1] = '\0';
            } else if (strcmp(key, "shutdown_command") == 0) {
                strncpy(shutdown_command, value + 1, sizeof(shutdown_command) - 1);
                shutdown_command[strlen(shutdown_command) - 1] = '\0';
            } else if (strcmp(key, "custom_no_gui_command") == 0) {
                strncpy(custom_no_gui_command, value + 1, sizeof(custom_no_gui_command) - 1);
                custom_no_gui_command[strlen(custom_no_gui_command) - 1] = '\0';
            }
        }
    }
    
    fclose(fp);

    if (strlen(custom_long_running_apps) == 0) {
        strncpy(custom_long_running_apps, DEFAULT_CUSTOM_LONG_RUNNING_APPS, sizeof(custom_long_running_apps) - 1);
    }

    if (strlen(sleep_command) == 0) {
        strncpy(sleep_command, DEFAULT_SLEEP_COMMAND, sizeof(sleep_command) - 1);
    }
    
    if (strlen(shutdown_command) == 0) {
        strncpy(shutdown_command, DEFAULT_SHUTDOWN_COMMAND, sizeof(shutdown_command) - 1);
    }
    
    if (strlen(custom_no_gui_command) == 0) {
        strncpy(custom_no_gui_command, DEFAULT_CUSTOM_NO_GUI_COMMAND, sizeof(custom_no_gui_command) - 1);
    }
    
    custom_long_running_apps[sizeof(custom_long_running_apps) - 1] = '\0';
    sleep_command[sizeof(sleep_command) - 1] = '\0';
    shutdown_command[sizeof(shutdown_command) - 1] = '\0';
    custom_no_gui_command[sizeof(custom_no_gui_command) - 1] = '\0';
}

void save_config(void) {
    ensure_config_dir_exists();
    
    char config_path[MAX_PATH_LENGTH];
    snprintf(config_path, sizeof(config_path), "%s/.XorgIdleManager/config.ini", getenv("HOME"));
    FILE *fp = fopen(config_path, "w");
    if (fp == NULL) {
        char error_message[MAX_ERROR_LENGTH];
        snprintf(error_message, sizeof(error_message), "Failed to open config file for writing: %s", strerror(errno));
        show_error(error_message);
        return;
    }

    fprintf(fp, "HOME=%s\n", getenv("HOME"));
    fprintf(fp, "inactivity_timeout=%d\n", inactivity_timeout);
    fprintf(fp, "long_running_apps_timeout=%d\n", long_running_apps_timeout);
    fprintf(fp, "custom_long_running_apps=%s\n", custom_long_running_apps);
    fprintf(fp, "check_interval=%d\n", check_interval);
    fprintf(fp, "cpu_threshold=%d\n", cpu_threshold);
    fprintf(fp, "io_threshold=%d\n", io_threshold);
    fprintf(fp, "net_threshold=%d\n", net_threshold);
    fprintf(fp, "cpu_net_check_interval=%d\n", cpu_net_check_interval);
    fprintf(fp, "io_check_interval=%d\n", io_check_interval);
    fprintf(fp, "boot_grace_period=%d\n", boot_grace_period);
    fprintf(fp, "action_type=%d\n", action_type);
    fprintf(fp, "sleep_command='%s'\n", sleep_command);
    fprintf(fp, "shutdown_command='%s'\n", shutdown_command);
    fprintf(fp, "custom_no_gui_command='%s'\n", custom_no_gui_command);

    fclose(fp);
}

int ms_to_s(int ms) {
    return ms / 1000;
}

int s_to_ms(int s) {
    return s * 1000;
}

void set_tooltip(GtkWidget *widget, const char *text) {
    gtk_widget_set_tooltip_text(widget, text);
}

GtkWidget *create_label_with_tooltip(const char *label_text, const char *tooltip_text) {
    GtkWidget *label = gtk_label_new(label_text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_tooltip_text(label, tooltip_text);
    return label;
}

void update_ui_values(void) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", ms_to_s(inactivity_timeout));
    gtk_entry_set_text(GTK_ENTRY(inactivity_timeout_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", ms_to_s(long_running_apps_timeout));
    gtk_entry_set_text(GTK_ENTRY(long_running_apps_timeout_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", check_interval);
    gtk_entry_set_text(GTK_ENTRY(check_interval_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", cpu_threshold);
    gtk_entry_set_text(GTK_ENTRY(cpu_threshold_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", io_threshold);
    gtk_entry_set_text(GTK_ENTRY(io_threshold_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", net_threshold);
    gtk_entry_set_text(GTK_ENTRY(net_threshold_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", cpu_net_check_interval);
    gtk_entry_set_text(GTK_ENTRY(cpu_net_check_interval_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", io_check_interval);
    gtk_entry_set_text(GTK_ENTRY(io_check_interval_entry), buffer);

    snprintf(buffer, sizeof(buffer), "%d", boot_grace_period);
    gtk_entry_set_text(GTK_ENTRY(boot_grace_period_entry), buffer);

    gtk_combo_box_set_active(GTK_COMBO_BOX(action_combo_box), action_type);

    gtk_entry_set_text(GTK_ENTRY(custom_long_running_apps_entry), custom_long_running_apps);
    gtk_entry_set_text(GTK_ENTRY(sleep_command_entry), sleep_command);
    gtk_entry_set_text(GTK_ENTRY(shutdown_command_entry), shutdown_command);
    gtk_entry_set_text(GTK_ENTRY(custom_no_gui_command_entry), custom_no_gui_command);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dpms_enabled_check), dpms_enabled);
    
    snprintf(buffer, sizeof(buffer), "%d", dpms_standby);
    gtk_entry_set_text(GTK_ENTRY(dpms_standby_entry), buffer);
    
    snprintf(buffer, sizeof(buffer), "%d", dpms_suspend);
    gtk_entry_set_text(GTK_ENTRY(dpms_suspend_entry), buffer);
    
    snprintf(buffer, sizeof(buffer), "%d", dpms_off);
    gtk_entry_set_text(GTK_ENTRY(dpms_off_entry), buffer);
}

void restore_defaults_sleep(GtkWidget *widget, gpointer data) {
    inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT_SLEEP;
    long_running_apps_timeout = DEFAULT_LONG_RUNNING_APPS_TIMEOUT_SLEEP;
    check_interval = DEFAULT_CHECK_INTERVAL;
    cpu_threshold = DEFAULT_CPU_THRESHOLD;
    io_threshold = DEFAULT_IO_THRESHOLD;
    net_threshold = DEFAULT_NET_THRESHOLD;
    cpu_net_check_interval = DEFAULT_CPU_NET_CHECK_INTERVAL;
    io_check_interval = DEFAULT_IO_CHECK_INTERVAL;
    action_type = DEFAULT_ACTION_TYPE_SLEEP;

    strncpy(custom_long_running_apps, DEFAULT_CUSTOM_LONG_RUNNING_APPS, sizeof(custom_long_running_apps) - 1);
    strncpy(sleep_command, DEFAULT_SLEEP_COMMAND, sizeof(sleep_command) - 1);
    custom_long_running_apps[sizeof(custom_long_running_apps) - 1] = '\0';
    sleep_command[sizeof(sleep_command) - 1] = '\0';

    update_ui_values();
}

void restore_defaults_shutdown(GtkWidget *widget, gpointer data) {
    inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT_SHUTDOWN;
    long_running_apps_timeout = DEFAULT_LONG_RUNNING_APPS_TIMEOUT_SHUTDOWN;
    check_interval = DEFAULT_CHECK_INTERVAL;
    cpu_threshold = DEFAULT_CPU_THRESHOLD;
    io_threshold = DEFAULT_IO_THRESHOLD;
    net_threshold = DEFAULT_NET_THRESHOLD;
    cpu_net_check_interval = DEFAULT_CPU_NET_CHECK_INTERVAL;
    io_check_interval = DEFAULT_IO_CHECK_INTERVAL;
    action_type = DEFAULT_ACTION_TYPE_SHUTDOWN;

    strncpy(custom_long_running_apps, DEFAULT_CUSTOM_LONG_RUNNING_APPS, sizeof(custom_long_running_apps) - 1);
    strncpy(shutdown_command, DEFAULT_SHUTDOWN_COMMAND, sizeof(shutdown_command) - 1);
    custom_long_running_apps[sizeof(custom_long_running_apps) - 1] = '\0';
    shutdown_command[sizeof(shutdown_command) - 1] = '\0';

    update_ui_values();
}

void restore_defaults_custom_no_gui(GtkWidget *widget, gpointer data) {
    inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT_NO_GUI;
    long_running_apps_timeout = DEFAULT_LONG_RUNNING_APPS_TIMEOUT_NO_GUI;
    check_interval = DEFAULT_CHECK_INTERVAL;
    cpu_threshold = DEFAULT_CPU_THRESHOLD;
    io_threshold = DEFAULT_IO_THRESHOLD;
    net_threshold = DEFAULT_NET_THRESHOLD;
    cpu_net_check_interval = DEFAULT_CPU_NET_CHECK_INTERVAL;
    io_check_interval = DEFAULT_IO_CHECK_INTERVAL;
    action_type = DEFAULT_ACTION_TYPE_SHUTDOWN;

    strncpy(custom_long_running_apps, DEFAULT_CUSTOM_LONG_RUNNING_APPS, sizeof(custom_long_running_apps) - 1);
    strncpy(custom_no_gui_command, DEFAULT_CUSTOM_NO_GUI_COMMAND, sizeof(custom_no_gui_command) - 1);
    custom_long_running_apps[sizeof(custom_long_running_apps) - 1] = '\0';
    custom_no_gui_command[sizeof(custom_no_gui_command) - 1] = '\0';

    update_ui_values();
}

int validate_script_command(const char *command) {
    if (!command || strlen(command) < 1) return 0;

    char *copy = strdup(command);
    if (!copy) return 0;

    int valid = 1;
    char *token = strtok(copy, ";");
    while (token && valid) {
        char *trimmed = token;
        while (*trimmed == ' ') trimmed++;

        if (strlen(trimmed) > 0) {
            if (strchr(trimmed, '|') || strchr(trimmed, '>') || strchr(trimmed, '<')) {
                valid = 0;
            }
        }
        token = strtok(NULL, ";");
    }

    free(copy);
    return valid;
}

void save_and_apply(GtkWidget *widget, gpointer data) {
    const char *inactivity_timeout_str = gtk_entry_get_text(GTK_ENTRY(inactivity_timeout_entry));
    const char *long_running_apps_timeout_str = gtk_entry_get_text(GTK_ENTRY(long_running_apps_timeout_entry));
    const char *check_interval_str = gtk_entry_get_text(GTK_ENTRY(check_interval_entry));
    const char *cpu_threshold_str = gtk_entry_get_text(GTK_ENTRY(cpu_threshold_entry));
    const char *io_threshold_str = gtk_entry_get_text(GTK_ENTRY(io_threshold_entry));
    const char *net_threshold_str = gtk_entry_get_text(GTK_ENTRY(net_threshold_entry));
    const char *cpu_net_check_interval_str = gtk_entry_get_text(GTK_ENTRY(cpu_net_check_interval_entry));
    const char *io_check_interval_str = gtk_entry_get_text(GTK_ENTRY(io_check_interval_entry));
    const char *boot_grace_period_str = gtk_entry_get_text(GTK_ENTRY(boot_grace_period_entry));
    const char *dpms_standby_str = gtk_entry_get_text(GTK_ENTRY(dpms_standby_entry));
    const char *dpms_suspend_str = gtk_entry_get_text(GTK_ENTRY(dpms_suspend_entry));
    const char *dpms_off_str = gtk_entry_get_text(GTK_ENTRY(dpms_off_entry));

    if (!validate_integer_input(inactivity_timeout_str, 1, INT_MAX) ||
        !validate_integer_input(long_running_apps_timeout_str, 1, INT_MAX) ||
        !validate_integer_input(check_interval_str, 1, INT_MAX) ||
        !validate_integer_input(cpu_threshold_str, 0, 100) ||
        !validate_integer_input(io_threshold_str, 0, INT_MAX) ||
        !validate_integer_input(net_threshold_str, 0, INT_MAX) ||
        !validate_integer_input(cpu_net_check_interval_str, 1, INT_MAX) ||
        !validate_integer_input(io_check_interval_str, 1, INT_MAX) ||
        !validate_integer_input(boot_grace_period_str, 1, 525600) ||
        !validate_integer_input(dpms_standby_str, 0, INT_MAX) ||
        !validate_integer_input(dpms_suspend_str, 0, INT_MAX) ||
        !validate_integer_input(dpms_off_str, 0, INT_MAX)) {
        show_error("Invalid input. Please check your values.");
        return;
    }

    inactivity_timeout = s_to_ms(atoi(inactivity_timeout_str));
    long_running_apps_timeout = s_to_ms(atoi(long_running_apps_timeout_str));
    check_interval = atoi(check_interval_str);
    cpu_threshold = atoi(cpu_threshold_str);
    io_threshold = atoi(io_threshold_str);
    net_threshold = atoi(net_threshold_str);
    cpu_net_check_interval = atoi(cpu_net_check_interval_str);
    io_check_interval = atoi(io_check_interval_str);
    boot_grace_period = atoi(boot_grace_period_str);
    action_type = gtk_combo_box_get_active(GTK_COMBO_BOX(action_combo_box));
    dpms_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dpms_enabled_check));
    dpms_standby = atoi(dpms_standby_str);
    dpms_suspend = atoi(dpms_suspend_str);
    dpms_off = atoi(dpms_off_str);

    const char *sleep_cmd = gtk_entry_get_text(GTK_ENTRY(sleep_command_entry));
    const char *shutdown_cmd = gtk_entry_get_text(GTK_ENTRY(shutdown_command_entry));
    const char *custom_long_running_apps_text = gtk_entry_get_text(GTK_ENTRY(custom_long_running_apps_entry));
    const char *custom_no_gui_cmd = gtk_entry_get_text(GTK_ENTRY(custom_no_gui_command_entry));

    strncpy(sleep_command, sleep_cmd, sizeof(sleep_command) - 1);
    strncpy(shutdown_command, shutdown_cmd, sizeof(shutdown_command) - 1);
    strncpy(custom_long_running_apps, custom_long_running_apps_text, sizeof(custom_long_running_apps) - 1);
    strncpy(custom_no_gui_command, custom_no_gui_cmd, sizeof(custom_no_gui_command) - 1);
    sleep_command[sizeof(sleep_command) - 1] = '\0';
    shutdown_command[sizeof(shutdown_command) - 1] = '\0';
    custom_long_running_apps[sizeof(custom_long_running_apps) - 1] = '\0';
    custom_no_gui_command[sizeof(custom_no_gui_command) - 1] = '\0';

    if (!validate_script_command(sleep_command)) {
        show_error("Invalid sleep command. Please check for potentially unsafe characters.");
        return;
    }

    if (!validate_script_command(shutdown_command)) {
        show_error("Invalid shutdown command. Please check for potentially unsafe characters.");
        return;
    }

    if (!validate_script_command(custom_no_gui_command)) {
        show_error("Invalid custom no-X-server command. Please check for potentially unsafe characters.");
        return;
    }

    save_config();

    if (dpms_enabled) {
        char *dpms_on_argv[] = {"xset", "+dpms", NULL};
        execute_command("xset", dpms_on_argv);

        char dpms_values[64];
        snprintf(dpms_values, sizeof(dpms_values), "%d %d %d", dpms_standby, dpms_suspend, dpms_off);

        char *dpms_set_argv[] = {"xset", "dpms", strdup(dpms_values), NULL};
        execute_command("xset", dpms_set_argv);
        free(dpms_set_argv[2]);
    } else {
        char *dpms_off_argv[] = {"xset", "-dpms", NULL};
        execute_command("xset", dpms_off_argv);
    }

    g_idle_add(restart_daemon_async, NULL);
}

gboolean restart_daemon_async(gpointer user_data) {
    system("kill -9 $(cat ~/.XorgIdleManager/xorg_idle_manager_pids 2>/dev/null) 2>/dev/null");
    system("rm -f ~/.XorgIdleManager/xorg_idle_manager_pids");
    int stop_result = system("pkill -f XorgIdleManager_daemon.sh");

    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        show_error("Failed to determine executable path.");
        return FALSE;
    }
    exe_path[len] = '\0';

    char *working_dir = g_path_get_dirname(exe_path);
    char *script_path = g_build_filename(working_dir, "XorgIdleManager_daemon.sh", NULL);

    FILE *script = fopen(script_path, "w");
    if (script == NULL) {
        show_error("Failed to create daemon script.");
        g_free(working_dir);
        g_free(script_path);
        return FALSE;
    }

    fprintf(script,
    "#!/bin/bash\n"
    "set -uo pipefail\n\n"
    "HOME=%s\n"
    "INACTIVITY_TIMEOUT=%d\n"
    "LONG_RUNNING_APPS_TIMEOUT=%d\n"
    "CUSTOM_LONG_RUNNING_APPS=\"%s\"\n"
    "CHECK_INTERVAL=%d\n"
    "CPU_THRESHOLD=%d\n"
    "IO_THRESHOLD=%d\n"
    "NET_THRESHOLD=%d\n"
    "CPU_NET_CHECK_INTERVAL=%d\n"
    "IO_CHECK_INTERVAL=%d\n"
    "BOOT_GRACE_PERIOD=%d\n"
    "ACTION_TYPE=%d\n"
    "SLEEP_COMMAND=\"set +e; %s\"\n"
    "SHUTDOWN_COMMAND=\"set +e; %s\"\n"
    "CUSTOM_NO_GUI_COMMAND=\"set +e; %s\"\n\n"

    "LOG_FILE=\"$HOME/.XorgIdleManager/xorg-idle-manager.log\"\n"
    "MAX_LOG_SIZE=$((30 * 1024))  # 30 KB\n\n"
    
    "PIDS_FILE=\"$HOME/.XorgIdleManager/xorg_idle_manager_pids\"\n\n"

    "if [ ! -d \"$HOME/.XorgIdleManager\" ]; then\n"
    "    mkdir -p \"$HOME/.XorgIdleManager\"\n"
    "    echo \"Directory $HOME/.XorgIdleManager created successfully\"\n"
    "fi\n\n"

    "truncate_log() {\n"
    "    if [ -f \"$LOG_FILE\" ] && [ $(LC_ALL=C stat -c%%s \"$LOG_FILE\") -gt $MAX_LOG_SIZE ]; then\n"
    "        LC_ALL=C tail -c $((MAX_LOG_SIZE / 2)) \"$LOG_FILE\" > \"$LOG_FILE.tmp\" && LC_ALL=C mv \"$LOG_FILE.tmp\" \"$LOG_FILE\"\n"
    "        echo \"$(LC_ALL=C date '+%%Y-%%m-%%d %%H:%%M:%%S') - Log file truncated to half size\" >> \"$LOG_FILE\"\n"
    "    fi\n"
    "}\n\n"

    "log() {\n"
    "    truncate_log\n"
    "    echo \"$(LC_ALL=C date '+%%Y-%%m-%%d %%H:%%M:%%S') - $1\" >> \"$LOG_FILE\"\n"
    "}\n\n"

    "truncate_log\n"
    "log \"=== Script started at $(LC_ALL=C date) ===\"\n\n"

    "check_command() {\n"
    "    LC_ALL=C command -v \"$1\" &> /dev/null\n"
    "}\n\n"

    "required_commands=(\"awk\" \"bc\" \"history\" \"grep\" \"iostat\" \"sleep\" \"xprintidle\")\n"
    "missing_commands=()\n"
    "for cmd in \"${required_commands[@]}\"; do\n"
    "    if ! check_command \"$cmd\"; then\n"
    "        missing_commands+=(\"$cmd\")\n"
    "    fi\n"
    "done\n\n"

    "if [ ${#missing_commands[@]} -ne 0 ]; then\n"
    "    log \"Error: The following required utilities are missing:\"\n"
    "    for cmd in \"${missing_commands[@]}\"; do\n"
    "        log \"  - $cmd\"\n"
    "    done\n"
    "    log \"Please install the missing utilities and try again.\"\n"
    "    exit 1\n"
    "fi\n\n"

    "log \"All required utilities are present. Starting the script...\"\n\n"

    "while [[ $# -gt 0 ]]; do\n"
    "    case $1 in\n"
    "        --home)\n"
    "            HOME=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --inactivity-timeout)\n"
    "            INACTIVITY_TIMEOUT=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --long-running-apps-timeout)\n"
    "            LONG_RUNNING_APPS_TIMEOUT=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --custom-long-running-apps)\n"
    "            CUSTOM_LONG_RUNNING_APPS=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --check-interval)\n"
    "            CHECK_INTERVAL=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --cpu-threshold)\n"
    "            CPU_THRESHOLD=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --io-threshold)\n"
    "            IO_THRESHOLD=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --net-threshold)\n"
    "            NET_THRESHOLD=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --cpu-net-check-interval)\n"
    "            CPU_NET_CHECK_INTERVAL=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --io-check-interval)\n"
    "            IO_CHECK_INTERVAL=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --boot_grace_period)\n"
    "            BOOT_GRACE_PERIOD=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --action-type)\n"
    "            ACTION_TYPE=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --sleep-command)\n"
    "            SLEEP_COMMAND=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --shutdown-command)\n"
    "            SHUTDOWN_COMMAND=\"$2\"\n"
    "            shift 2 ;;\n"
    "        --custom-no-gui-command)\n"
    "            CUSTOM_NO_GUI_COMMAND=\"$2\"\n"
    "            shift 2 ;;\n"
    "        *)\n"
    "            echo \"Unknown parameter: $1\"\n"
    "            exit 1 ;;\n"
    "    esac\n"
    "done\n\n"

    "is_system_booting() {\n"
    "    local uptime_seconds\n"
    "    uptime_seconds=$(awk '{print int($1)}' /proc/uptime)\n"
    "    local minutes=$((uptime_seconds / 60))\n"
    "\n"
    "    if [[ $minutes -lt $BOOT_GRACE_PERIOD ]]; then\n"
    "        log \"System is in boot grace period (${minutes} minutes < boot grace period of ${BOOT_GRACE_PERIOD} minutes)\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"System boot grace period has passed (boot grace period: ${BOOT_GRACE_PERIOD} minutes)\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"

    "check_gui() {\n"
    "    log \"Checking for GUI with X server\"\n"
    "    \n"
    "    if [ \"${XDG_SESSION_TYPE:-}\" = \"x11\" ]; then\n"
    "        log \"GUI detected via XDG_SESSION_TYPE=x11\"\n"
    "        return 0\n"
    "    fi\n"
    "    \n"
    "    if [ -n \"${DISPLAY:-}\" ] && command -v xset >/dev/null; then\n"
    "        if timeout 5 xset q >/dev/null 2>&1; then\n"
    "            log \"X server is active (DISPLAY=$DISPLAY)\"\n"
    "            return 0\n"
    "        else\n"
    "            log \"X server DISPLAY exists but unresponsive\"\n"
    "            return 1\n"
    "        fi\n"
    "    fi\n"
    "    \n"
    "    if timeout 5 pgrep -x \"Xorg\" >/dev/null; then\n"
    "        log \"Xorg process detected\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"No Xorg process found\"\n"
    "    fi\n"
    "    \n"
    "    if timeout 5 pgrep -f \"xfce|mate-session|lxsession|cinnamon|openbox|fluxbox|i3|awesome|bspwm|dwm|xmonad|qtile|herbstluftwm|jwm|icewm|blackbox|fvwm|twm|gnome-shell|kwin_wayland|sway\" >/dev/null; then\n"
    "        log \"GUI environment detected, but not X server\"\n"
    "    fi\n"
    "    \n"
    "    log \"No active X server found\"\n"
    "    return 1\n"
    "}\n\n"

    "check_no_gui_user_activity() {\n"
    "    local tty=\"/dev/pts/$(tty | awk -F'/' '{print $NF}')\"\n"
    "    local last_update=$(stat -c %Y \"$tty\" 2>/dev/null || echo 0)\n"
    "    local current_time=$(cat /proc/uptime | awk '{print int($1)}')\n\n"
    "    local inactivity_timeout_seconds=$((INACTIVITY_TIMEOUT / 1000))\n\n"
    "    if [ $((current_time - last_update)) -lt $inactivity_timeout_seconds ]; then\n"
    "        log \"User activity detected based on TTY interaction.\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"No user activity detected.\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"

    "check_cpu_activity() {\n"
    "    log \"Checking CPU activity\"\n"
    "    read total1 idle1 < <(LC_ALL=C awk '/cpu / {print $2+$3+$4+$5+$6+$7+$8, $5}' /proc/stat)\n"
    "    LC_ALL=C sleep $CPU_NET_CHECK_INTERVAL\n"
    "    read total2 idle2 < <(LC_ALL=C awk '/cpu / {print $2+$3+$4+$5+$6+$7+$8, $5}' /proc/stat)\n\n"
    "    local total_diff=$((total2 - total1))\n"
    "    [ $total_diff -eq 0 ] && total_diff=1\n"
    "    local idle_diff=$((idle2 - idle1))\n"
    "    local cpu_usage=$(echo \"scale=2; 100 * ($total_diff - $idle_diff) / $total_diff\" | bc)\n"
    "    cpu_usage=$(echo \"if ($cpu_usage < 0) 0 else $cpu_usage\" | bc)\n"
    "    cpu_usage=${cpu_usage/#./0.}\n"
    "    log \"CPU usage: ${cpu_usage}%\"\n"
    "    if (( $(echo \"${cpu_usage} > $CPU_THRESHOLD\" | bc -l) )); then\n"
    "        log \"High CPU activity detected: ${cpu_usage}% (threshold: $CPU_THRESHOLD%)\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"CPU activity below threshold: ${cpu_usage}% (threshold: $CPU_THRESHOLD%)\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"

    "check_io_activity() {\n"
    "    log \"Checking I/O activity\"\n"
    "    log \"Running iostat command with interval: $IO_CHECK_INTERVAL\"\n"
    "    local iostat_output\n"
    "    iostat_output=$(LC_ALL=C timeout $((IO_CHECK_INTERVAL * 2 + 1)) iostat -d -k -y $IO_CHECK_INTERVAL 1 2>&1)\n"
    "    local exit_status=$?\n"
    "    if [ $exit_status -eq 124 ]; then\n"
    "        log \"iostat command timed out\"\n"
    "        return 1\n"
    "    elif [ $exit_status -ne 0 ]; then\n"
    "        log \"iostat command failed with exit status $exit_status\"\n"
    "        return 1\n"
    "    fi\n"
    "    log \"iostat command completed. Output: $iostat_output\"\n"
    "    \n"
    "    local read_activity=$(echo \"$iostat_output\" | awk '/^[a-z]/ && $3 ~ /^[0-9.]+$/ {sum_read += $3} END {print sum_read+0}')\n"
    "    local write_activity=$(echo \"$iostat_output\" | awk '/^[a-z]/ && $4 ~ /^[0-9.]+$/ {sum_write += $4} END {print sum_write+0}')\n"
    "    local io_activity=$(echo \"$read_activity + $write_activity\" | bc)\n"
    "    log \"Read activity: $read_activity KB/s, Write activity: $write_activity KB/s\"\n"
    "    log \"Total I/O activity: $io_activity KB/s\"\n"
    "    \n"
    "    if (( $(echo \"$io_activity > $IO_THRESHOLD\" | bc -l) )); then\n"
    "        log \"High I/O activity detected: $io_activity KB/s (threshold: $IO_THRESHOLD KB/s)\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"I/O activity below threshold: $io_activity KB/s (threshold: $IO_THRESHOLD KB/s)\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"

    "check_network_activity() {\n"
    "    log \"Checking network activity\"\n"
    "    local start_values end_values net_activity\n"
    "    read -r start_values < <(LC_ALL=C awk '$1 ~ /^[[:alnum:]]+:$/ && $1 !~ /^lo:/ {sum += $2 + $10} END {print sum+0}' /proc/net/dev)\n"
    "    LC_ALL=C sleep $CPU_NET_CHECK_INTERVAL\n"
    "    read -r end_values < <(LC_ALL=C awk '$1 ~ /^[[:alnum:]]+:$/ && $1 !~ /^lo:/ {sum += $2 + $10} END {print sum+0}' /proc/net/dev)\n"
    "    net_activity=$(echo \"scale=1; ($end_values - $start_values) / ($CPU_NET_CHECK_INTERVAL * 1024)\" | bc)\n"
    "    net_activity=$(echo \"if ($net_activity < 0) 0 else $net_activity\" | bc)\n"
    "    net_activity=${net_activity/#./0.}\n"
    "    log \"Network activity: ${net_activity} KB/s\"\n"
    "    if (( $(echo \"$net_activity > $NET_THRESHOLD\" | bc -l) )); then\n"
    "        log \"High network activity detected: ${net_activity} KB/s (threshold: $NET_THRESHOLD KB/s)\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"Network activity below threshold: ${net_activity} KB/s (threshold: $NET_THRESHOLD KB/s)\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"

    "check_system_activity() {\n"
    "    log \"Checking system activity\"\n"
    "    if check_cpu_activity || check_io_activity || check_network_activity; then\n"
    "        log \"System activity detected\"\n"
    "        return 0\n"
    "    else\n"
    "        log \"No significant system activity detected\"\n"
    "        return 1\n"
    "    fi\n"
    "}\n\n"
    
    "check_long_running_apps_active() {\n"
    "    local current_pid=$$\n"
    "    LC_ALL=C ps -eo pid,comm= | grep -E \"($CUSTOM_LONG_RUNNING_APPS)\" | grep -v \"^$current_pid \" > /dev/null\n"
    "}\n"

    "perform_action() {\n"
    "    local environment=\"$1\"\n"
    "    \n"
    "    if [ \"$environment\" = \"no-gui\" ]; then\n"
    "        if [ -n \"$CUSTOM_NO_GUI_COMMAND\" ]; then\n"
    "            log \"Executing Custom No X Server Command: $CUSTOM_NO_GUI_COMMAND\"\n"
    "            eval \"$CUSTOM_NO_GUI_COMMAND\"\n"
    "        else\n"
    "            log \"Warning: No custom command defined for no-X-server environment. No action taken.\"\n"
    "        fi\n"
    "    else\n"
    "        if [ \"$ACTION_TYPE\" -eq 0 ]; then\n"
    "            if [ -n \"$SLEEP_COMMAND\" ]; then\n"
    "                log \"Executing GUI Sleep Command: $SLEEP_COMMAND\"\n"
    "                eval \"$SLEEP_COMMAND\"\n"
    "            else\n"
    "                log \"Warning: No sleep command defined for X-server environment. No action taken.\"\n"
    "            fi\n"
    "        else\n"
    "            if [ -n \"$SHUTDOWN_COMMAND\" ]; then\n"
    "                log \"Executing GUI Shutdown Command: $SHUTDOWN_COMMAND\"\n"
    "                eval \"$SHUTDOWN_COMMAND\"\n"
    "            else\n"
    "                log \"Warning: No shutdown command defined for X-server environment. No action taken.\"\n"
    "            fi\n"
    "        fi\n"
    "    fi\n"
    "}\n\n"

    "cleanup_pids() {\n"
    "    [ -f \"$PIDS_FILE\" ] && \n"
    "    awk '{ if (system(\"ps -p \" $1 \" >/dev/null 2>&1\") == 0) print $1 }' \"$PIDS_FILE\" > \"$PIDS_FILE.tmp\" && \n"
    "    mv \"$PIDS_FILE.tmp\" \"$PIDS_FILE\"\n"
    "}\n"

    "limit_pids() {\n"
    "    [ -f \"$PIDS_FILE\" ] && tail -n 50 \"$PIDS_FILE\" > \"$PIDS_FILE.tmp\" && mv \"$PIDS_FILE.tmp\" \"$PIDS_FILE\"\n"
    "}\n"

    "echo $$ > \"$PIDS_FILE\"\n"
    "log \"Main script PID saved to $PIDS_FILE\"\n"

    "log \"Entering main loop\"\n\n"

    " while true; do\n"
    "     for cmd in awk bc grep iostat; do\n"
    "         pgrep -P $$ -f \"$cmd\" >> \"$PIDS_FILE\"\n"
    "     done\n"
    "\n"
    "     cleanup_pids\n"
    "\n"
    "     log \"Starting main loop iteration.\"\n"
    "     if is_system_booting; then\n"
    "         log \"System is booting, skipping activity check this iteration\"\n"
    "         log \"Sleeping for ${CHECK_INTERVAL} seconds.\"\n"
    "         LC_ALL=C sleep $CHECK_INTERVAL & echo $! >> \"$PIDS_FILE\"\n"
    "         wait $!\n"
    "         cleanup_pids\n"
    "         limit_pids\n"
    "         continue\n"
    "     fi\n"
    "     \n"
    "     if check_gui; then\n"
    "         log \"X server detected. Using standard logic.\"\n"
    "         user_inactive_time=$(LC_ALL=C xprintidle)\n"
    "         if check_long_running_apps_active; then\n"
    "             timeout=$LONG_RUNNING_APPS_TIMEOUT\n"
    "             extended_timeout_seconds=$(echo \"scale=2; $LONG_RUNNING_APPS_TIMEOUT / 1000\" | bc)\n"
    "             log \"Long-running apps active. Using extended timeout: ${extended_timeout_seconds} seconds\"\n"
    "         else\n"
    "             timeout=$INACTIVITY_TIMEOUT\n"
    "             standard_timeout_seconds=$(echo \"scale=2; $INACTIVITY_TIMEOUT / 1000\" | bc)\n"
    "             log \"No long-running apps active. Using standard timeout: ${standard_timeout_seconds} seconds\"\n"
    "         fi\n"
    "         \n"
    "         inactive_time_seconds=$(echo \"scale=2; $user_inactive_time / 1000\" | bc)\n"
    "         current_timeout_seconds=$(echo \"scale=2; $timeout / 1000\" | bc)\n"
    "         inactive_time_seconds=$(sed 's/^\\./0./' <<< \"$inactive_time_seconds\")\n"
    "         log \"User inactive time: ${inactive_time_seconds} seconds (current timeout: ${current_timeout_seconds} seconds).\"\n"
    "         \n"
    "         if (( user_inactive_time < timeout )); then\n"
    "             log \"User activity detected (screen active). Skipping system activity check.\"\n"
    "         elif ! check_system_activity; then\n"
    "             log \"Inactivity timeout reached and no system activity detected. Initiating X-server action.\"\n"
    "             perform_action \"gui\"\n"
    "         else\n"
    "             log \"System activity detected despite user inactivity. Continuing to monitor.\"\n"
    "         fi\n"
    "     else\n"
    "         log \"No X server detected. Using no-X-server user activity check.\"\n"
    "         if check_no_gui_user_activity; then\n"
    "             log \"User activity detected in no-X-server environment. Resetting timer.\"\n"
    "         elif ! check_system_activity; then\n"
    "             log \"No user activity and no system activity detected in no-X-server environment. Initiating no-X-server action.\"\n"
    "             perform_action \"no-gui\"\n"
    "         else\n"
    "             log \"System activity detected in no-X-server environment. Continuing to monitor.\"\n"
    "         fi\n"
    "     fi\n"
    "     \n"
    "     log \"Sleeping for ${CHECK_INTERVAL} seconds.\"\n"
    "     LC_ALL=C sleep $CHECK_INTERVAL & echo $! >> \"$PIDS_FILE\"\n"
    "     wait $!\n"
    "     cleanup_pids\n"
    "     limit_pids\n"
    " done\n",

    getenv("HOME"),
    inactivity_timeout,
    long_running_apps_timeout,
    custom_long_running_apps,
    check_interval,
    cpu_threshold,
    io_threshold,
    net_threshold,
    cpu_net_check_interval,
    io_check_interval,
    boot_grace_period,
    action_type,
    sleep_command,
    shutdown_command,
    custom_no_gui_command);

    fclose(script);
    chmod(script_path, 0755);

    pid_t pid = fork();
    if (pid == -1) {
        show_error("Failed to fork process for starting daemon.");
        g_free(working_dir);
        g_free(script_path);
        return FALSE;
    }

    if (pid == 0) {
        setsid();
        execl("/bin/bash", "/bin/bash", script_path, NULL);
        _exit(EXIT_FAILURE);
    }

    g_free(working_dir);
    g_free(script_path);

    if (stop_result == 0) {
        show_info("Configuration saved and daemon restarted successfully.");
    } else {
        show_info("Configuration saved and daemon started successfully.");
    }

    return FALSE;
}

GtkWidget *create_button(const char *label, GCallback callback) {
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, NULL);
    return button;
}

void show_license_info(GtkWidget *widget, gpointer data) {
    const char *license_text = "GNU GENERAL PUBLIC LICENSE\n\n"
"Version 3, 29 June 2007\n\n"
"Copyright © 2007 Free Software Foundation, Inc. <https://fsf.org/>\n\n"
"Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.\n\n"
"Preamble\n\n"
"The GNU General Public License is a free, copyleft license for software and other kinds of works.\n\n"
"The licenses for most software and other practical works are designed to take away your freedom to share and change the works. By contrast, the GNU General Public License is intended to guarantee your freedom to share and change all versions of a program--to make sure it remains free software for all its users. We, the Free Software Foundation, use the GNU General Public License for most of our software; it applies also to any other work released this way by its authors. You can apply it to your programs, too.\n\n"
"When we speak of free software, we are referring to freedom, not price. Our General Public Licenses are designed to make sure that you have the freedom to distribute copies of free software (and charge for them if you wish), that you receive source code or can get it if you want it, that you can change the software or use pieces of it in new free programs, and that you know you can do these things.\n\n"
"To protect your rights, we need to prevent others from denying you these rights or asking you to surrender the rights. Therefore, you have certain responsibilities if you distribute copies of the software, or if you modify it: responsibilities to respect the freedom of others.\n\n"
"For example, if you distribute copies of such a program, whether gratis or for a fee, you must pass on to the recipients the same freedoms that you received. You must make sure that they, too, receive or can get the source code. And you must show them these terms so they know their rights.\n\n"
"Developers that use the GNU GPL protect your rights with two steps: (1) assert copyright on the software, and (2) offer you this License giving you legal permission to copy, distribute and/or modify it.\n\n"
"For the developers' and authors' protection, the GPL clearly explains that there is no warranty for this free software. For both users' and authors' sake, the GPL requires that modified versions be marked as changed, so that their problems will not be attributed erroneously to authors of previous versions.\n\n"
"Some devices are designed to deny users access to install or run modified versions of the software inside them, although the manufacturer can do so. This is fundamentally incompatible with the aim of protecting users' freedom to change the software. The systematic pattern of such abuse occurs in the area of products for individuals to use, which is precisely where it is most unacceptable. Therefore, we have designed this version of the GPL to prohibit the practice for those products. If such problems arise substantially in other domains, we stand ready to extend this provision to those domains in future versions of the GPL, as needed to protect the freedom of users.\n\n"
"Finally, every program is threatened constantly by software patents. States should not allow patents to restrict development and use of software on general-purpose computers, but in those that do, we wish to avoid the special danger that patents applied to a free program could make it effectively proprietary. To prevent this, the GPL assures that patents cannot be used to render the program non-free.\n\n"
"The precise terms and conditions for copying, distribution and modification follow.\n\n"
"TERMS AND CONDITIONS\n\n"
"0. Definitions.\n\n"
"This License” refers to version 3 of the GNU General Public License.\n\n"
"“Copyright” also means copyright-like laws that apply to other kinds of works, such as semiconductor masks.\n\n"
"“The Program” refers to any copyrightable work licensed under this License. Each licensee is addressed as “you”. “Licensees” and “recipients” may be individuals or organizations.\n\n"
"To “modify” a work means to copy from or adapt all or part of the work in a fashion requiring copyright permission, other than the making of an exact copy. The resulting work is called a “modified version” of the earlier work or a work “based on” the earlier work.\n\n"
"A “covered work” means either the unmodified Program or a work based on the Program.\n\n"
"To “propagate” a work means to do anything with it that, without permission, would make you directly or secondarily liable for infringement under applicable copyright law, except executing it on a computer or modifying a private copy. Propagation includes copying, distribution (with or without modification), making available to the public, and in some countries other activities as well.\n\n"
"To “convey” a work means any kind of propagation that enables other parties to make or receive copies. Mere interaction with a user through a computer network, with no transfer of a copy, is not conveying.\n\n"
"An interactive user interface displays “Appropriate Legal Notices” to the extent that it includes a convenient and prominently visible feature that (1) displays an appropriate copyright notice, and (2) tells the user that there is no warranty for the work (except to the extent that warranties are provided), that licensees may convey the work under this License, and how to view a copy of this License. If the interface presents a list of user commands or options, such as a menu, a prominent item in the list meets this criterion.\n\n"
"1. Source Code.\n\n"
"The “source code” for a work means the preferred form of the work for making modifications to it. “Object code” means any non-source form of a work.\n\n"
"A “Standard Interface” means an interface that either is an official standard defined by a recognized standards body, or, in the case of interfaces specified for a particular programming language, one that is widely used among developers working in that language.\n\n"
"The “System Libraries” of an executable work include anything, other than the work as a whole, that (a) is included in the normal form of packaging a Major Component, but which is not part of that Major Component, and (b) serves only to enable use of the work with that Major Component, or to implement a Standard Interface for which an implementation is available to the public in source code form. A “Major Component”, in this context, means a major essential component (kernel, window system, and so on) of the specific operating system (if any) on which the executable work runs, or a compiler used to produce the work, or an object code interpreter used to run it.\n\n"
"The “Corresponding Source” for a work in object code form means all the source code needed to generate, install, and (for an executable work) run the object code and to modify the work, including scripts to control those activities. However, it does not include the work's System Libraries, or general-purpose tools or generally available free programs which are used unmodified in performing those activities but which are not part of the work. For example, Corresponding Source includes interface definition files associated with source files for the work, and the source code for shared libraries and dynamically linked subprograms that the work is specifically designed to require, such as by intimate data communication or control flow between those subprograms and other parts of the work.\n\n"
"The Corresponding Source need not include anything that users can regenerate automatically from other parts of the Corresponding Source.\n\n"
"The Corresponding Source for a work in source code form is that same work.\n\n"
"2. Basic Permissions.\n\n"
"All rights granted under this License are granted for the term of copyright on the Program, and are irrevocable provided the stated conditions are met. This License explicitly affirms your unlimited permission to run the unmodified Program. The output from running a covered work is covered by this License only if the output, given its content, constitutes a covered work. This License acknowledges your rights of fair use or other equivalent, as provided by copyright law.\n\n"
"You may make, run and propagate covered works that you do not convey, without conditions so long as your license otherwise remains in force. You may convey covered works to others for the sole purpose of having them make modifications exclusively for you, or provide you with facilities for running those works, provided that you comply with the terms of this License in conveying all material for which you do not control copyright. Those thus making or running the covered works for you must do so exclusively on your behalf, under your direction and control, on terms that prohibit them from making any copies of your copyrighted material outside their relationship with you.\n\n"
"Conveying under any other circumstances is permitted solely under the conditions stated below. Sublicensing is not allowed; section 10 makes it unnecessary.\n\n"
"3. Protecting Users' Legal Rights From Anti-Circumvention Law.\n\n"
"No covered work shall be deemed part of an effective technological measure under any applicable law fulfilling obligations under article 11 of the WIPO copyright treaty adopted on 20 December 1996, or similar laws prohibiting or restricting circumvention of such measures.\n\n"
"When you convey a covered work, you waive any legal power to forbid circumvention of technological measures to the extent such circumvention is effected by exercising rights under this License with respect to the covered work, and you disclaim any intention to limit operation or modification of the work as a means of enforcing, against the work's users, your or third parties' legal rights to forbid circumvention of technological measures.\n\n"
"4. Conveying Verbatim Copies.\n\n"
"You may convey verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice; keep intact all notices stating that this License and any non-permissive terms added in accord with section 7 apply to the code; keep intact all notices of the absence of any warranty; and give all recipients a copy of this License along with the Program.\n\n"
"You may charge any price or no price for each copy that you convey, and you may offer support or warranty protection for a fee.\n\n"
"5. Conveying Modified Source Versions.\n\n"
"You may convey a work based on the Program, or the modifications to produce it from the Program, in the form of source code under the terms of section 4, provided that you also meet all of these conditions:\n\n"
"    a) The work must carry prominent notices stating that you modified it, and giving a relevant date.\n\n"
"    b) The work must carry prominent notices stating that it is released under this License and any conditions added under section 7. This requirement modifies the requirement in section 4 to “keep intact all notices”.\n\n"
"    c) You must license the entire work, as a whole, under this License to anyone who comes into possession of a copy. This License will therefore apply, along with any applicable section 7 additional terms, to the whole of the work, and all its parts, regardless of how they are packaged. This License gives no permission to license the work in any other way, but it does not invalidate such permission if you have separately received it.\n\n"
"    d) If the work has interactive user interfaces, each must display Appropriate Legal Notices; however, if the Program has interactive interfaces that do not display Appropriate Legal Notices, your work need not make them do so.\n\n"
"A compilation of a covered work with other separate and independent works, which are not by their nature extensions of the covered work, and which are not combined with it such as to form a larger program, in or on a volume of a storage or distribution medium, is called an “aggregate” if the compilation and its resulting copyright are not used to limit the access or legal rights of the compilation's users beyond what the individual works permit. Inclusion of a covered work in an aggregate does not cause this License to apply to the other parts of the aggregate.\n\n"
"6. Conveying Non-Source Forms.\n\n"
"You may convey a covered work in object code form under the terms of sections 4 and 5, provided that you also convey the machine-readable Corresponding Source under the terms of this License, in one of these ways:\n\n"
"    a) Convey the object code in, or embodied in, a physical product (including a physical distribution medium), accompanied by the Corresponding Source fixed on a durable physical medium customarily used for software interchange.\n\n"
"    b) Convey the object code in, or embodied in, a physical product (including a physical distribution medium), accompanied by a written offer, valid for at least three years and valid for as long as you offer spare parts or customer support for that product model, to give anyone who possesses the object code either (1) a copy of the Corresponding Source for all the software in the product that is covered by this License, on a durable physical medium customarily used for software interchange, for a price no more than your reasonable cost of physically performing this conveying of source, or (2) access to copy the Corresponding Source from a network server at no charge.\n\n"
"    c) Convey individual copies of the object code with a copy of the written offer to provide the Corresponding Source. This alternative is allowed only occasionally and noncommercially, and only if you received the object code with such an offer, in accord with subsection 6b.\n\n"
"    d) Convey the object code by offering access from a designated place (gratis or for a charge), and offer equivalent access to the Corresponding Source in the same way through the same place at no further charge. You need not require recipients to copy the Corresponding Source along with the object code. If the place to copy the object code is a network server, the Corresponding Source may be on a different server (operated by you or a third party) that supports equivalent copying facilities, provided you maintain clear directions next to the object code saying where to find the Corresponding Source. Regardless of what server hosts the Corresponding Source, you remain obligated to ensure that it is available for as long as needed to satisfy these requirements.\n\n"
"    e) Convey the object code using peer-to-peer transmission, provided you inform other peers where the object code and Corresponding Source of the work are being offered to the general public at no charge under subsection 6d.\n\n"
"A separable portion of the object code, whose source code is excluded from the Corresponding Source as a System Library, need not be included in conveying the object code work.\n\n"
"A “User Product” is either (1) a “consumer product”, which means any tangible personal property which is normally used for personal, family, or household purposes, or (2) anything designed or sold for incorporation into a dwelling. In determining whether a product is a consumer product, doubtful cases shall be resolved in favor of coverage. For a particular product received by a particular user, “normally used” refers to a typical or common use of that class of product, regardless of the status of the particular user or of the way in which the particular user actually uses, or expects or is expected to use, the product. A product is a consumer product regardless of whether the product has substantial commercial, industrial or non-consumer uses, unless such uses represent the only significant mode of use of the product.\n\n"
"“Installation Information” for a User Product means any methods, procedures, authorization keys, or other information required to install and execute modified versions of a covered work in that User Product from a modified version of its Corresponding Source. The information must suffice to ensure that the continued functioning of the modified object code is in no case prevented or interfered with solely because modification has been made.\n\n"
"If you convey an object code work under this section in, or with, or specifically for use in, a User Product, and the conveying occurs as part of a transaction in which the right of possession and use of the User Product is transferred to the recipient in perpetuity or for a fixed term (regardless of how the transaction is characterized), the Corresponding Source conveyed under this section must be accompanied by the Installation Information. But this requirement does not apply if neither you nor any third party retains the ability to install modified object code on the User Product (for example, the work has been installed in ROM).\n\n"
"The requirement to provide Installation Information does not include a requirement to continue to provide support service, warranty, or updates for a work that has been modified or installed by the recipient, or for the User Product in which it has been modified or installed. Access to a network may be denied when the modification itself materially and adversely affects the operation of the network or violates the rules and protocols for communication across the network.\n\n"
"Corresponding Source conveyed, and Installation Information provided, in accord with this section must be in a format that is publicly documented (and with an implementation available to the public in source code form), and must require no special password or key for unpacking, reading or copying.\n\n"
"7. Additional Terms.\n\n"
"“Additional permissions” are terms that supplement the terms of this License by making exceptions from one or more of its conditions. Additional permissions that are applicable to the entire Program shall be treated as though they were included in this License, to the extent that they are valid under applicable law. If additional permissions apply only to part of the Program, that part may be used separately under those permissions, but the entire Program remains governed by this License without regard to the additional permissions.\n\n"
"When you convey a copy of a covered work, you may at your option remove any additional permissions from that copy, or from any part of it. (Additional permissions may be written to require their own removal in certain cases when you modify the work.) You may place additional permissions on material, added by you to a covered work, for which you have or can give appropriate copyright permission.\n\n"
"Notwithstanding any other provision of this License, for material you add to a covered work, you may (if authorized by the copyright holders of that material) supplement the terms of this License with terms:\n\n"
"    a) Disclaiming warranty or limiting liability differently from the terms of sections 15 and 16 of this License; or\n\n"
"    b) Requiring preservation of specified reasonable legal notices or author attributions in that material or in the Appropriate Legal Notices displayed by works containing it; or\n\n"
"    c) Prohibiting misrepresentation of the origin of that material, or requiring that modified versions of such material be marked in reasonable ways as different from the original version; or\n\n"
"    d) Limiting the use for publicity purposes of names of licensors or authors of the material; or\n\n"
"    e) Declining to grant rights under trademark law for use of some trade names, trademarks, or service marks; or\n\n"
"    f) Requiring indemnification of licensors and authors of that material by anyone who conveys the material (or modified versions of it) with contractual assumptions of liability to the recipient, for any liability that these contractual assumptions directly impose on those licensors and authors.\n\n"
"All other non-permissive additional terms are considered “further restrictions” within the meaning of section 10. If the Program as you received it, or any part of it, contains a notice stating that it is governed by this License along with a term that is a further restriction, you may remove that term. If a license document contains a further restriction but permits relicensing or conveying under this License, you may add to a covered work material governed by the terms of that license document, provided that the further restriction does not survive such relicensing or conveying.\n\n"
"If you add terms to a covered work in accord with this section, you must place, in the relevant source files, a statement of the additional terms that apply to those files, or a notice indicating where to find the applicable terms.\n\n"
"Additional terms, permissive or non-permissive, may be stated in the form of a separately written license, or stated as exceptions; the above requirements apply either way.\n\n"
"8. Termination.\n\n"
"You may not propagate or modify a covered work except as expressly provided under this License. Any attempt otherwise to propagate or modify it is void, and will automatically terminate your rights under this License (including any patent licenses granted under the third paragraph of section 11).\n\n"
"However, if you cease all violation of this License, then your license from a particular copyright holder is reinstated (a) provisionally, unless and until the copyright holder explicitly and finally terminates your license, and (b) permanently, if the copyright holder fails to notify you of the violation by some reasonable means prior to 60 days after the cessation.\n\n"
"Moreover, your license from a particular copyright holder is reinstated permanently if the copyright holder notifies you of the violation by some reasonable means, this is the first time you have received notice of violation of this License (for any work) from that copyright holder, and you cure the violation prior to 30 days after your receipt of the notice.\n\n"
"Termination of your rights under this section does not terminate the licenses of parties who have received copies or rights from you under this License. If your rights have been terminated and not permanently reinstated, you do not qualify to receive new licenses for the same material under section 10.\n\n"
"9. Acceptance Not Required for Having Copies.\n\n"
"You are not required to accept this License in order to receive or run a copy of the Program. Ancillary propagation of a covered work occurring solely as a consequence of using peer-to-peer transmission to receive a copy likewise does not require acceptance. However, nothing other than this License grants you permission to propagate or modify any covered work. These actions infringe copyright if you do not accept this License. Therefore, by modifying or propagating a covered work, you indicate your acceptance of this License to do so.\n\n"
"10. Automatic Licensing of Downstream Recipients.\n\n"
"Each time you convey a covered work, the recipient automatically receives a license from the original licensors, to run, modify and propagate that work, subject to this License. You are not responsible for enforcing compliance by third parties with this License.\n\n"
"An “entity transaction” is a transaction transferring control of an organization, or substantially all assets of one, or subdividing an organization, or merging organizations. If propagation of a covered work results from an entity transaction, each party to that transaction who receives a copy of the work also receives whatever licenses to the work the party's predecessor in interest had or could give under the previous paragraph, plus a right to possession of the Corresponding Source of the work from the predecessor in interest, if the predecessor has it or can get it with reasonable efforts.\n\n"
"You may not impose any further restrictions on the exercise of the rights granted or affirmed under this License. For example, you may not impose a license fee, royalty, or other charge for exercise of rights granted under this License, and you may not initiate litigation (including a cross-claim or counterclaim in a lawsuit) alleging that any patent claim is infringed by making, using, selling, offering for sale, or importing the Program or any portion of it.\n\n"
"11. Patents.\n\n"
"A “contributor” is a copyright holder who authorizes use under this License of the Program or a work on which the Program is based. The work thus licensed is called the contributor's “contributor version”.\n\n"
"A contributor's “essential patent claims” are all patent claims owned or controlled by the contributor, whether already acquired or hereafter acquired, that would be infringed by some manner, permitted by this License, of making, using, or selling its contributor version, but do not include claims that would be infringed only as a consequence of further modification of the contributor version. For purposes of this definition, “control” includes the right to grant patent sublicenses in a manner consistent with the requirements of this License.\n\n"
"Each contributor grants you a non-exclusive, worldwide, royalty-free patent license under the contributor's essential patent claims, to make, use, sell, offer for sale, import and otherwise run, modify and propagate the contents of its contributor version.\n\n"
"In the following three paragraphs, a “patent license” is any express agreement or commitment, however denominated, not to enforce a patent (such as an express permission to practice a patent or covenant not to sue for patent infringement). To “grant” such a patent license to a party means to make such an agreement or commitment not to enforce a patent against the party.\n\n"
"If you convey a covered work, knowingly relying on a patent license, and the Corresponding Source of the work is not available for anyone to copy, free of charge and under the terms of this License, through a publicly available network server or other readily accessible means, then you must either (1) cause the Corresponding Source to be so available, or (2) arrange to deprive yourself of the benefit of the patent license for this particular work, or (3) arrange, in a manner consistent with the requirements of this License, to extend the patent license to downstream recipients. “Knowingly relying” means you have actual knowledge that, but for the patent license, your conveying the covered work in a country, or your recipient's use of the covered work in a country, would infringe one or more identifiable patents in that country that you have reason to believe are valid.\n\n"
"If, pursuant to or in connection with a single transaction or arrangement, you convey, or propagate by procuring conveyance of, a covered work, and grant a patent license to some of the parties receiving the covered work authorizing them to use, propagate, modify or convey a specific copy of the covered work, then the patent license you grant is automatically extended to all recipients of the covered work and works based on it.\n\n"
"A patent license is “discriminatory” if it does not include within the scope of its coverage, prohibits the exercise of, or is conditioned on the non-exercise of one or more of the rights that are specifically granted under this License. You may not convey a covered work if you are a party to an arrangement with a third party that is in the business of distributing software, under which you make payment to the third party based on the extent of your activity of conveying the work, and under which the third party grants, to any of the parties who would receive the covered work from you, a discriminatory patent license (a) in connection with copies of the covered work conveyed by you (or copies made from those copies), or (b) primarily for and in connection with specific products or compilations that contain the covered work, unless you entered into that arrangement, or that patent license was granted, prior to 28 March 2007.\n\n"
"Nothing in this License shall be construed as excluding or limiting any implied license or other defenses to infringement that may otherwise be available to you under applicable patent law.\n\n"
"12. No Surrender of Others' Freedom.\n\n"
"If conditions are imposed on you (whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License. If you cannot convey a covered work so as to satisfy simultaneously your obligations under this License and any other pertinent obligations, then as a consequence you may not convey it at all. For example, if you agree to terms that obligate you to collect a royalty for further conveying from those to whom you convey the Program, the only way you could satisfy both those terms and this License would be to refrain entirely from conveying the Program.\n\n"
"13. Use with the GNU Affero General Public License.\n\n"
"Notwithstanding any other provision of this License, you have permission to link or combine any covered work with a work licensed under version 3 of the GNU Affero General Public License into a single combined work, and to convey the resulting work. The terms of this License will continue to apply to the part which is the covered work, but the special requirements of the GNU Affero General Public License, section 13, concerning interaction through a network will apply to the combination as such.\n\n"
"14. Revised Versions of this License.\n\n"
"The Free Software Foundation may publish revised and/or new versions of the GNU General Public License from time to time. Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns.\n\n"
"Each version is given a distinguishing version number. If the Program specifies that a certain numbered version of the GNU General Public License “or any later version” applies to it, you have the option of following the terms and conditions either of that numbered version or of any later version published by the Free Software Foundation. If the Program does not specify a version number of the GNU General Public License, you may choose any version ever published by the Free Software Foundation.\n\n"
"If the Program specifies that a proxy can decide which future versions of the GNU General Public License can be used, that proxy's public statement of acceptance of a version permanently authorizes you to choose that version for the Program.\n\n"
"Later license versions may give you additional or different permissions. However, no additional obligations are imposed on any author or copyright holder as a result of your choosing to follow a later version.\n\n"
"15. Disclaimer of Warranty.\n\n"
"THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n"
"16. Limitation of Liability.\n\n"
"IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\n\n"
"17. Interpretation of Sections 15 and 16.\n\n"
"If the disclaimer of warranty and limitation of liability provided above cannot be given local legal effect according to their terms, reviewing courts shall apply local law that most closely approximates an absolute waiver of all civil liability in connection with the Program, unless a warranty or assumption of liability accompanies a copy of the Program in return for a fee.\n\n"
"END OF TERMS AND CONDITIONS\n\n"
"How to Apply These Terms to Your New Programs\n\n"
"If you develop a new program, and you want it to be of the greatest possible use to the public, the best way to achieve this is to make it free software which everyone can redistribute and change under these terms.\n\n"
"To do so, attach the following notices to the program. It is safest to attach them to the start of each source file to most effectively state the exclusion of warranty; and each file should have at least the “copyright” line and a pointer to where the full notice is found.\n\n"
"XorgIdleManager: a utility designed to manage system sleep or shutdown based on inactivity in X11 environments.\n"
"Copyright (C) 2025 Maksym Nazar.\n"
"Created with the assistance of Perplexity, ChatGPT and Claude.\n\n"
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n\n"
"For inquiries, please contact us at:\n"
"Email: maximkursua@gmail.com\n";

    GtkWidget *license_dialog;
    GtkWidget *content_area;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    license_dialog = gtk_dialog_new_with_buttons("License Information",
                                                 GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                 GTK_DIALOG_MODAL,
                                                 "Close", GTK_RESPONSE_CLOSE,
                                                 NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(license_dialog));
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 640, 400);

    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, license_text, -1);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);

    gtk_widget_show_all(license_dialog);

    gtk_dialog_run(GTK_DIALOG(license_dialog));
    gtk_widget_destroy(license_dialog);
}

void show_usage_terms(GtkWidget *widget, gpointer data) {
    const char *terms_text = "TERMS OF USE\n\n"
"1. Usage:\n"
"You are granted a non-exclusive, non-transferable license to use the Program under the terms of the GNU General Public License (GPL) Version 3.0. The term \"Program\" refers to the software package or product distributed under this License. You may use, copy, modify, and distribute the Program freely, provided that all copies and derivative works are licensed under the GPL and include this license notice.\n\n"
"2. License:\n"
"This Program is licensed under the GNU General Public License (GPL) Version 3.0, which ensures that users have the freedom to run, study, share, and modify the software. A copy of the GPL license is included with the Program package, or you can access it at https://www.gnu.org/licenses/gpl-3.0.html.\n\n"
"3. Source Code Availability:\n"
"As required by the GNU General Public License (GPL), the full source code of this Program is available and can be obtained from the official repository or package distribution. If you did not receive a copy of the source code, you may request it from the developer. Additionally, you have the right to access and modify the source code under the terms of this License.\n\n"
"4. Disclaimer of Warranties:\n"
"The Program is provided \"as is,\" without any warranties, express or implied, including but not limited to the implied warranties of merchantability or fitness for a particular purpose. The developers make no representations or warranties regarding the use or performance of the Program.\n\n"
"5. Limitation of Liability:\n"
"In no event shall the developers be liable for any direct, indirect, incidental, special, exemplary, or consequential damages, including but not limited to damages for loss of data or profit, arising out of or in connection with the use of or inability to use the Program, even if advised of the possibility of such damages.\n\n"
"6. Modifications to the Program:\n"
"You may modify and distribute modified versions of the Program, provided you comply with the terms of the GNU General Public License (GPL). The developers reserve the right to modify, update, or discontinue the Program at their discretion.\n\n"
"7. Compliance with Laws:\n"
"You are responsible for complying with all applicable local, state, national, and international laws in connection with your use of the Program.\n\n"
"8. Copyright:\n"
"Copyright (C) 2025 Maksym Nazar.\n"
"Created with the assistance of Perplexity, ChatGPT and Claude.\n"
"This work is licensed under the GNU General Public License (GPL) Version 3.0.\n\n"
"9. Contact:\n"
"For inquiries, please contact us at:\n"
"Email: maximkursua@gmail.com\n";

    GtkWidget *terms_dialog;
    GtkWidget *content_area;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    terms_dialog = gtk_dialog_new_with_buttons("Terms of Use",
                                               GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                               GTK_DIALOG_MODAL,
                                               "Close", GTK_RESPONSE_CLOSE,
                                               NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(terms_dialog));
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 640, 400);

    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, terms_text, -1);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);

    gtk_widget_show_all(terms_dialog);

    gtk_dialog_run(GTK_DIALOG(terms_dialog));
    gtk_widget_destroy(terms_dialog);
}

void generate_shell_command(GtkWidget *widget, gpointer data) {
    char command[MAX_COMMAND_LENGTH * 2];
    char executable_path[PATH_MAX];
    char *last_slash;

    if (realpath("/proc/self/exe", executable_path) == NULL) {
        perror("Error getting executable path");
        return;
    }

    last_slash = strrchr(executable_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
    }

    char script_path[PATH_MAX];
    snprintf(script_path, sizeof(script_path), "%s/XorgIdleManager_daemon.sh", executable_path);

    snprintf(command, sizeof(command), 
        "\"%s\" --inactivity-timeout %d --long-running-apps-timeout %d --check-interval %d "
        "--cpu-threshold %d --io-threshold %d --net-threshold %d --cpu-net-check-interval %d "
        "--io-check-interval %d --action-type %d --sleep-command '%s' --shutdown-command '%s' "
        "--custom-long-running-apps '%s'",
        script_path,
        inactivity_timeout, long_running_apps_timeout, check_interval,
        cpu_threshold, io_threshold, net_threshold, 
        cpu_net_check_interval, io_check_interval, 
        action_type, sleep_command, shutdown_command, custom_long_running_apps);

    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    dialog = gtk_dialog_new_with_buttons(
        "Shell Command",
        GTK_WINDOW(gtk_widget_get_toplevel(widget)),
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL
    );

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 600, 200);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, command, -1);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_help_and_info(GtkWidget *widget, gpointer data) {
    const char *info = "XorgIdleManager v1.5: Purpose and Usage\n\n"
                       "======================== Purpose: ========================\n"
                       "XorgIdleManager is designed to manage system sleep or shutdown based on inactivity in X11 environments.\n\n"
                       "======================= How it works: ======================\n"
                       "1. The program runs a background daemon process that monitors system activity.\n"
                       "2. It uses various parameters to determine when the system is idle.\n"
                       "3. When the system is determined to be idle, it executes a predefined action – either a Sleep Command or a Shutdown Command, depending on the user's configuration.\n\n"
                       "======================= Parameters: =======================\n"
                       "- Inactivity Timeout: The time (in seconds) of overall user inactivity after which the system will perform the selected action (sleep or shutdown).\n"
                       "- Long-Running Applications Timeout: A separate timeout (in seconds) for when any of the specified long-running applications are active. This overrides the general inactivity timeout.\n"
                       "- Custom Long-Running Applications: List of processes (separated by '|') that will use the special Long-Running Applications Timeout. This can include media players, torrent clients, video editors, or any other applications that require extended periods of inactivity.\n"
                       "- Check Interval: The main interval (in seconds) at which the program performs its overall inactivity check. For example, if set to 3600 seconds (1 hour), the program will run a complete check every hour.\n"
                       "- CPU Threshold: The percentage of CPU usage above which the system is considered active.\n"
                       "- I/O Threshold: The amount of disk I/O activity (in KB/s) above which the system is considered active.\n"
                       "- Network Threshold: The amount of network activity (in KB/s) above which the system is considered active.\n"
                       "- CPU/Net Check Interval: The interval (in seconds) between two measurements of CPU and network activity.\n"
                       "- I/O Check Interval: The duration (in seconds) for which disk I/O activity is measured.\n"
                       "- Boot Grace Period: The time (in minutes) after system boot during which no actions will be performed by the idle manager. This prevents unintended sleep or shutdown immediately after boot if XorgIdleManager_Daemon is set to autostart.\n"
                       "- DPMS Enable: Enable or disable DPMS (Display Power Management Signaling).\n"
                       "- DPMS Standby: Time in seconds before the monitor enters standby mode.\n"
                       "- DPMS Suspend: Time in seconds before the monitor enters suspend mode.\n"
                       "- DPMS Off: Time in seconds before the monitor turns off.\n\n"
                       "======================== Actions: ========================\n"
                       "- Sleep Command: This can be a single command or a macro command (a sequence of commands) executed to put the system into sleep mode while running in an X server environment. These include a default macro command provided by XorgIdleManager, which users can replace with their own custom command or macro command, if needed.\n"
                       "- Shutdown Command: Similarly, this can be either a single command or a macro command executed to shut down the system while running in an X server environment. Users can rely on the default option provided by XorgIdleManager or customize it, if needed.\n"
                       "- Custom No X Server Command: The command to execute when the system is idle and no X server is detected. This allows users to define specific actions for non-X11 environments or headless setups.\n\n"
                       "============ DPMS (Display Power Management Signaling): ============\n"
                       "DPMS is used to manage power consumption of the display. Note that modern monitors may handle all DPMS states (standby, suspend, and off) identically. The effectiveness of these settings may vary depending on your specific hardware.\n\n"
                       "======== Running the Background Process With Custom Parameters: ========\n"
                       "To run the background daemon process with parameters other than those defined by the user in the configuration, use the \"Generate Daemon Command\" function. This function allows you to create a command for starting the daemon with custom parameters. Modify the necessary parameters in the generated command and execute it in the terminal.\n\n"
                       "=============== Autostarting the Background Process: ===============\n"
                       "You can add the background daemon process to your system's startup script or autostart configuration file. The daemon does not require a GUI to operate and will run independently in the background. If you need to use custom parameters for autostart, apply the \"Generate Daemon Command\" function to create a command that suits your needs and include it in your autostart configuration.\n\n"
                       "====================== Notes for Users ======================\n"
                       "1. Please be aware that the actual execution of sleep and shutdown commands may not occur exactly at the specified Inactivity Timeout (or Long-Running Applications Timeout) and Check Interval. The program performs additional checks, specifically CPU/Net Check Interval and I/O Check Interval, which take time as defined in the settings. Additionally, the overall system load and speed can affect the timing. Therefore, the actual time before action may be longer than the set parameters.\n"
                       "2. The program automatically determines and saves the user's home directory in the configuration file when using \"Save and Apply.\" This ensures that scripts launched before user authorization (e.g., during system autostart) work correctly even if the system's dynamic HOME variable is not yet initialized. Such functionality prevents potential issues with script execution in environments where user-specific variables are not yet set.\n";


    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    dialog = gtk_dialog_new_with_buttons("Help and Information",
                                         GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                         GTK_DIALOG_MODAL,
                                         "Close", GTK_RESPONSE_CLOSE,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, 640, 400);

    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, info, -1);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled_window);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void stop_daemon(GtkWidget *widget, gpointer data) {
    const char *home_directory = getenv("HOME");
    if (home_directory == NULL) {
        show_info("Environment variable HOME is not defined.");
        return;
    }

    FILE *fp = popen("pgrep -f XorgIdleManager_daemon.sh", "r");
    if (fp == NULL) {
        show_info("Error checking daemon status.");
        return;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        pclose(fp);

        char command[512];

        snprintf(command, sizeof(command), "kill -9 $(cat %s/.XorgIdleManager/xorg_idle_manager_pids 2>/dev/null) 2>/dev/null", home_directory);
        system(command);

        snprintf(command, sizeof(command), "rm -f %s/.XorgIdleManager/xorg_idle_manager_pids", home_directory);
        system(command);

        system("pkill -f XorgIdleManager_daemon.sh");

        show_info("XorgIdleManager daemon stopped.");
    } else {
        pclose(fp);
        show_info("XorgIdleManager daemon is not running.");
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "XorgIdleManager Configuration");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 651, 711);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    set_window_icon(window);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);
    gtk_widget_set_hexpand(grid, TRUE);

    inactivity_timeout_entry = gtk_entry_new();
    long_running_apps_timeout_entry = gtk_entry_new();
    custom_long_running_apps_entry = gtk_entry_new();
    check_interval_entry = gtk_entry_new();
    cpu_threshold_entry = gtk_entry_new();
    io_threshold_entry = gtk_entry_new();
    net_threshold_entry = gtk_entry_new();
    cpu_net_check_interval_entry = gtk_entry_new();
    io_check_interval_entry = gtk_entry_new();
    boot_grace_period_entry = gtk_entry_new();
    sleep_command_entry = gtk_entry_new();
    shutdown_command_entry = gtk_entry_new();
    custom_no_gui_command_entry = gtk_entry_new();

    gtk_widget_set_hexpand(inactivity_timeout_entry, TRUE);
    gtk_widget_set_hexpand(long_running_apps_timeout_entry, TRUE);
    gtk_widget_set_hexpand(custom_long_running_apps_entry, TRUE);
    gtk_widget_set_hexpand(check_interval_entry, TRUE);
    gtk_widget_set_hexpand(cpu_threshold_entry, TRUE);
    gtk_widget_set_hexpand(io_threshold_entry, TRUE);
    gtk_widget_set_hexpand(net_threshold_entry, TRUE);
    gtk_widget_set_hexpand(cpu_net_check_interval_entry, TRUE);
    gtk_widget_set_hexpand(io_check_interval_entry, TRUE);
    gtk_widget_set_hexpand(boot_grace_period_entry, TRUE);
    gtk_widget_set_hexpand(sleep_command_entry, TRUE);
    gtk_widget_set_hexpand(shutdown_command_entry, TRUE);
    gtk_widget_set_hexpand(custom_no_gui_command_entry, TRUE);

    action_combo_box = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(action_combo_box), "Sleep");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(action_combo_box), "Shutdown");
    gtk_widget_set_hexpand(action_combo_box, TRUE);

    dpms_enabled_check = gtk_check_button_new_with_label("Enable DPMS");
    dpms_standby_entry = gtk_entry_new();
    dpms_suspend_entry = gtk_entry_new();
    dpms_off_entry = gtk_entry_new();

    gtk_widget_set_hexpand(dpms_standby_entry, TRUE);
    gtk_widget_set_hexpand(dpms_suspend_entry, TRUE);
    gtk_widget_set_hexpand(dpms_off_entry, TRUE);

    int row = 0;

    #define ADD_ENTRY_WITH_LABEL(label_text, tooltip_text, entry) \
    { \
        GtkWidget *label = create_label_with_tooltip(label_text, tooltip_text); \
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1); \
        gtk_grid_attach(GTK_GRID(grid), entry, 1, row, 3, 1); \
        gtk_widget_set_tooltip_text(entry, tooltip_text); \
        row++; \
    }

    ADD_ENTRY_WITH_LABEL("Inactivity Timeout (s):", "Inactivity Timeout: The time (in seconds) of overall user inactivity after which the system will perform the selected action (sleep or shutdown).", inactivity_timeout_entry)
    ADD_ENTRY_WITH_LABEL("Long-Running Apps Timeout (s):", "Long-Running Applications Timeout: A separate timeout (in seconds) for when any of the specified long-running applications are active. This overrides the general inactivity timeout.", long_running_apps_timeout_entry)
    ADD_ENTRY_WITH_LABEL("Custom Long-Running Applications:", "Custom Long-Running Applications: List of processes (separated by '|') that will use the special Long-Running Applications Timeout. This can include media players, torrent clients, video editors, or any other applications that require extended periods of inactivity.", custom_long_running_apps_entry)
    ADD_ENTRY_WITH_LABEL("Check Interval (s):", "Check Interval: The main interval (in seconds) at which the program performs its overall inactivity check. For example, if set to 3600 seconds (1 hour), the program will run a complete check every hour.", check_interval_entry)
    ADD_ENTRY_WITH_LABEL("CPU Threshold (%):", "CPU Threshold: The percentage of CPU usage above which the system is considered active.", cpu_threshold_entry)
    ADD_ENTRY_WITH_LABEL("I/O Threshold (KB/s):", "I/O Threshold: The amount of disk I/O activity (in KB/s) above which the system is considered active.", io_threshold_entry)
    ADD_ENTRY_WITH_LABEL("Network Threshold (KB/s):", "Network Threshold: The amount of network activity (in KB/s) above which the system is considered active.", net_threshold_entry)
    ADD_ENTRY_WITH_LABEL("CPU/Net Check Interval (s):", "CPU/Net Check Interval: The interval (in seconds) between two measurements of CPU and network activity.", cpu_net_check_interval_entry)
    ADD_ENTRY_WITH_LABEL("I/O Check Interval (s):", "I/O Check Interval: The interval (in seconds) between two I/O measurements for calculating average disk activity.\nNote: IO_CHECK_INTERVAL defines the time interval between the initial and final measurements, not the total duration of this check.", io_check_interval_entry)
    ADD_ENTRY_WITH_LABEL("Boot Grace Period (min):", "The time (in minutes) after system boot during which the idle manager will not perform any actions. This parameter is important to prevent the system from shutting down or going to sleep immediately after boot if XorgIdleManager_Daemon is set to autostart.", boot_grace_period_entry)

    GtkWidget *action_label = create_label_with_tooltip("Action:", "Action to perform after inactivity (sleep or shutdown).");
    gtk_grid_attach(GTK_GRID(grid), action_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), action_combo_box, 1, row, 3, 1);
    gtk_widget_set_tooltip_text(action_combo_box, "Action to perform after inactivity (sleep or shutdown).");
    row++;

    ADD_ENTRY_WITH_LABEL("Sleep Command:", "The command to execute when the system goes to sleep while running under an X server environment.", sleep_command_entry)
    ADD_ENTRY_WITH_LABEL("Shutdown Command:", "The command to execute when the system shuts down while running under an X server environment.", shutdown_command_entry)
    ADD_ENTRY_WITH_LABEL("Custom No X Server Command:", "The command to execute when system is idle and no X server is detected.", custom_no_gui_command_entry)

    gtk_grid_attach(GTK_GRID(grid), dpms_enabled_check, 0, row, 4, 1);
    gtk_widget_set_tooltip_text(dpms_enabled_check, "Enable or disable DPMS (Display Power Management Signaling)");
    row++;

    ADD_ENTRY_WITH_LABEL("DPMS Standby:", "DPMS (Display Power Management Signaling) Standby: Time in seconds before the monitor enters standby mode.\nNote: Modern monitors may handle all DPMS states (standby, suspend, and off) identично.", dpms_standby_entry)
    ADD_ENTRY_WITH_LABEL("DPMS Suspend:", "DPMS (Display Power Management Signaling) Suspend: Time in seconds before the monitor enters suspend mode.\nNote: Modern monitors may handle all DPMS states (standby, suspend, and off) identично.", dpms_suspend_entry)
    ADD_ENTRY_WITH_LABEL("DPMS Off:", "DPMS (Display Power Management Signaling) Off: Time in seconds before the monitor turns off.\nNote: Modern monitors may handle all DPMS states (standby, suspend, and off) identично.", dpms_off_entry)

    #undef ADD_ENTRY_WITH_LABEL

    GtkWidget *button_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(button_grid), 5);
    gtk_grid_set_row_spacing(GTK_GRID(button_grid), 5);
    gtk_widget_set_halign(button_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button_grid, GTK_ALIGN_END);

    GtkWidget *save_button = create_button("Save and Apply", G_CALLBACK(save_and_apply));
    GtkWidget *generate_command_button = create_button("Generate Daemon Command", G_CALLBACK(generate_shell_command));
    GtkWidget *stop_daemon_button = create_button("Stop XorgIdleManager Daemon", G_CALLBACK(stop_daemon));
    GtkWidget *defaults_sleep_button = create_button("Restore Defaults (Sleep)", G_CALLBACK(restore_defaults_sleep));
    GtkWidget *defaults_shutdown_button = create_button("Restore Defaults (Shutdown)", G_CALLBACK(restore_defaults_shutdown));
    GtkWidget *defaults_custom_no_gui_button = create_button("Restore Defaults (No X Server)", G_CALLBACK(restore_defaults_custom_no_gui));
    GtkWidget *help_button = create_button("Help and Information", G_CALLBACK(show_help_and_info));
    GtkWidget *terms_button = create_button("Terms of Use", G_CALLBACK(show_usage_terms));
    GtkWidget *license_button = create_button("License", G_CALLBACK(show_license_info));
    GtkWidget *cancel_button = create_button("Cancel", G_CALLBACK(gtk_main_quit));

    gtk_grid_attach(GTK_GRID(button_grid), save_button, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), generate_command_button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), stop_daemon_button, 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(button_grid), defaults_sleep_button, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), defaults_shutdown_button, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), defaults_custom_no_gui_button, 2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(button_grid), help_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), terms_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(button_grid), license_button, 2, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(button_grid), cancel_button, 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), button_grid, 0, row, 4, 1);

    read_config();
    get_dpms_settings();
    update_ui_values();

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

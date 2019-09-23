#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#define WATCH_DIR     "/var/bigbluebutton/published/presentation/"
#define INOTIFY_DELAY 10
#define EVENT_SIZE    sizeof(struct inotify_event)
#define BUFFER_LEN    1024 * (EVENT_SIZE + 16)

int main() {

	int fd = inotify_init();
	if (fd < 0) {
		perror("inotify_init");
	}

	int wd = inotify_add_watch(fd, WATCH_DIR, IN_CREATE);
	printf(" * Watching %s\n", WATCH_DIR);

	int length, i;
	char buffer[BUFFER_LEN];
	char file_path[256];
	char curl_buffer[512];

	// Infinite loop
	while (1) {
		length = read(fd, buffer, BUFFER_LEN);
		if (length < 0) {
			perror("read");
		}

		i = 0;
		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event -> len) {
				sleep(INOTIFY_DELAY); // have a delay to be sure the dir is completely copied
				printf(" * The file/dir %s was created.\n", event -> name);

				strcpy(file_path, WATCH_DIR);
				strcat(file_path, event -> name);
				strcat(file_path, "/metadata.xml");

				if (access(file_path, F_OK) == -1) {
					printf(" - The metadata file did not exist. %s\n", file_path);
				} else {
					printf(" * The metadata file exist. Sending the notification.\n");

					// Send the notification via Telegram Bot API
					strcpy(curl_buffer, "curl 'https://api.telegram.org/bottokenhere/sendMessage?parse_mode=HTML&chat_id=-1001173113661&text=<b>S2</b> -> <b>bbb-watch</b> -> Presentation got published. ID: ");
					strcat(curl_buffer, event -> name);
					strcat(curl_buffer, " <a href=\"https://s2.novinparsian.ir/playback/presentation/2.0/playback.html?meetingId=");
					strcat(curl_buffer, event -> name);
					strcat(curl_buffer, "\">Link</a>'");

					system(curl_buffer);
				}

			}
			i += EVENT_SIZE + event -> len;
		}
	}
}

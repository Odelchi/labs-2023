[Unit]
Description=Userspace application, interacting with character driver
[Service]
User=root
Environment=TERM=xterm
ExecStart=/bin/chardev
Restart=always   
[Install]
WantedBy=multi-user.target

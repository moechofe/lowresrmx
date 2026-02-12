#!/bin/bash

BACKUP_DIR="/srv/lowresrmx/redis-backup"
REDIS_DIR="/var/lib/redis"
REDIS_RDB="dump.rdb"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="redis_backup_${DATE}.rdb"
LOCKFILE="/var/run/redis-backup.lock"

# Cleanup function to remove lock on exit
cleanup() {
    echo "Cleaning up..."
    rm -f "$LOCKFILE"
    exit
}

# Trap all exit scenarios including CTRL-C
trap cleanup EXIT SIGINT SIGTERM

# Check if another instance is running
if [ -f "$LOCKFILE" ]; then
    PID=$(cat "$LOCKFILE")
    if ps -p "$PID" > /dev/null 2>&1; then
        echo "Backup already running (PID: $PID). Exiting."
        exit 1
    else
        echo "Stale lock file found. Removing."
        rm -f "$LOCKFILE"
    fi
fi

# Create lock file
echo $$ > "$LOCKFILE"

# Trigger background save
redis-cli BGSAVE

# Wait for BGSAVE to complete
echo "Waiting for BGSAVE to complete..."
INITIAL_SAVE=$(redis-cli LASTSAVE)
sleep 2

# Poll until LASTSAVE timestamp changes
MAX_WAIT=300  # 5 minutes timeout
WAITED=0
while [ $WAITED -lt $MAX_WAIT ]; do
    CURRENT_SAVE=$(redis-cli LASTSAVE)
    if [ "$CURRENT_SAVE" -gt "$INITIAL_SAVE" ]; then
        echo "BGSAVE completed"
        break
    fi
    sleep 2
    WAITED=$((WAITED + 2))
done

if [ $WAITED -ge $MAX_WAIT ]; then
    echo "Error: BGSAVE timeout after ${MAX_WAIT} seconds"
    exit 1
fi

# Wait a bit more to ensure file is fully written
sleep 2

# Copy the RDB file
if [ -f "${REDIS_DIR}/${REDIS_RDB}" ]; then
    cp "${REDIS_DIR}/${REDIS_RDB}" "${BACKUP_DIR}/${BACKUP_FILE}"
    lz4 -9 "${BACKUP_DIR}/${BACKUP_FILE}" "${BACKUP_DIR}/${BACKUP_FILE}.lz4"
    rm "${BACKUP_DIR}/${BACKUP_FILE}"
    echo "Backup created: ${BACKUP_DIR}/${BACKUP_FILE}.lz4"
else
    echo "Error: RDB file not found"
    exit 1
fi

# Set permissions
chown redis:redis "${BACKUP_DIR}/${BACKUP_FILE}.lz4"
chmod 640 "${BACKUP_DIR}/${BACKUP_FILE}.lz4"

echo "Redis backup completed successfully"

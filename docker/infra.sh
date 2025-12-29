#!/usr/bin/env sh

set -e

COMPOSE_FILE="docker-compose.yml"

usage() {
  echo "Usage: $0 {up|down|down-clean|restart|status}"
  echo
  echo "Commands:"
  echo "  up           Start containers (volumes preserved)"
  echo "  down         Stop containers (volumes preserved)"
  echo "  down-clean   Stop containers and REMOVE volumes"
  echo "  restart      Restart containers (volumes preserved)"
  echo "  status       Show containers status"
}

require_compose() {
  if ! command -v docker >/dev/null 2>&1; then
    echo "Docker is not installed"
    exit 1
  fi

  if ! docker compose version >/dev/null 2>&1; then
    echo "Docker Compose plugin is not available"
    exit 1
  fi
}

confirm_volume_removal() {
  echo "This will DELETE all Docker volumes (Kafka & PostgreSQL data)"
  printf "Are you sure? [y/N]: "
  read ans
  case "$ans" in
    y|Y|yes|YES) ;;
    *) echo "Aborted"; exit 0 ;;
  esac
}

require_compose

case "$1" in
  up)
    echo "Starting infrastructure"
    docker compose -f "$COMPOSE_FILE" up -d
    ;;

  down)
    echo "Stopping infrastructure (volumes preserved)"
    docker compose -f "$COMPOSE_FILE" down
    ;;

  down-clean)
    confirm_volume_removal
    echo "Stopping infrastructure and removing volumes"
    docker compose -f "$COMPOSE_FILE" down -v
    ;;

  restart)
    echo "Restarting infrastructure"
    docker compose -f "$COMPOSE_FILE" down
    docker compose -f "$COMPOSE_FILE" up -d
    ;;

  status)
    docker compose -f "$COMPOSE_FILE" ps
    ;;

  *)
    usage
    exit 1
    ;;
esac

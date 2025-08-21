# syntax=docker/dockerfile:1

FROM ghcr.io/astral-sh/uv:python3.13-alpine

ENV LANG="C.UTF-8" \
    TZ=Asia/Shanghai \
    UV_COMPILE_BYTECODE=1

WORKDIR /app

ADD py .
RUN uv sync --frozen --no-cache

ENV PATH="/app/.venv/bin:$PATH"

CMD ["uv", "run", "main.py", "-c", "/app/config"]

EXPOSE 28299
VOLUME [ "/app/config" ]

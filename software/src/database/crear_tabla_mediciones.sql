
-- Tabla de mediciones del ESP32

CREATE TABLE IF NOT EXISTS public.mediciones (
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    dispositivo TEXT NOT NULL,

    fecha_hora TIMESTAMPTZ NOT NULL,

    epoch_utc BIGINT NOT NULL,

    conector_1 DOUBLE PRECISION,
    conector_2 DOUBLE PRECISION,
    conector_3 DOUBLE PRECISION,
    conector_4 DOUBLE PRECISION,

    recibido_en TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT mediciones_dispositivo_epoch_unique
        UNIQUE (dispositivo, epoch_utc)
);

-- Acelera consultas por dispositivo y fecha.

CREATE INDEX IF NOT EXISTS idx_mediciones_dispositivo_fecha
ON public.mediciones (dispositivo, fecha_hora DESC);

-- Activar Row Level Security.

ALTER TABLE public.mediciones
ENABLE ROW LEVEL SECURITY;

-- No crear politicas de acceso publico.
-- La Edge Function escribira mediante una clave
-- de servidor, que nunca se entrega al ESP32.

SELECT
    RPAD(
        CAST(soc.id_socio AS CHAR),
        19,
        ' '
        ) AS idSocio,
	cci.valor AS monto,
    CONCAT(
        LPAD(cci.mes, 2, '0'),
        '/',
        cci.anio
    ) AS periodo,
    cci.anio AS anioPeriodo,
    cci.mes AS mesPeriodo,
    cci.fecha_emision AS emision,
    date_format(DATE_ADD(cci.fecha_emision, INTERVAL 10 DAY), '%Y%m%d') AS vencimiento,
    date_format(DATE_ADD(cci.fecha_emision, INTERVAL 15 DAY), '%Y%m%d') AS vencimientoDos,
    date_format(DATE_ADD(cci.fecha_emision, INTERVAL 20 DAY), '%Y%m%d') AS vencimientoTres,
    RPAD(
        cci.id_cta_cte_item,
        20,
        ' '
        ) AS idFactura,
    LPAD(
        REPLACE(CAST(ROUND(cci.valor,2) AS CHAR),'.',''),
        11,
        '0'
    ) AS montoFormat


FROM io_socio soc
JOIN io_estado_socio es ON es.id_estado_socio = soc.estado_socio_id
JOIN io_cta_cte cc ON cc.socio_id = soc.id_socio
JOIN io_cta_cte_item cci ON cci.cta_cte_id = cc.id_cta_cte
LEFT JOIN io_categoria_socio cs ON cs.id_categoria_socio = cci.categoria_id
LEFT JOIN io_tipo_grupo_familiar tgf ON tgf.id_tipo_grupo_familiar = cci.tipo_grupo_familiar_id
LEFT JOIN io_categoria_socio cs2 ON cs2.id_categoria_socio = cs.id_categoria_socio
LEFT JOIN io_tipo_grupo_familiar tgf2 ON tgf2.id_tipo_grupo_familiar = tgf.id_tipo_grupo_familiar
WHERE es.socio_activo = TRUE
	AND cci.anulado = FALSE AND cci.refinanciado = FALSE AND cci.pagado = FALSE
    AND cci.concepto_id = 1
ORDER BY cci.anio DESC, cci.mes DESC;
--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: elements; Type: TABLE; Schema: public; Owner: postgres; Tablespace: 
--

CREATE TABLE elements (
    modelid integer NOT NULL,
    id integer NOT NULL,
    type character varying,
    data text
);


ALTER TABLE public.elements OWNER TO postgres;

--
-- Name: layouts; Type: TABLE; Schema: public; Owner: postgres; Tablespace: 
--

CREATE TABLE layouts (
    modelid integer NOT NULL,
    id integer NOT NULL,
    type character varying,
    data text
);


ALTER TABLE public.layouts OWNER TO postgres;

--
-- Name: models; Type: TABLE; Schema: public; Owner: postgres; Tablespace: 
--

CREATE TABLE models (
    id integer NOT NULL,
    name character varying(255),
    owner character varying(255),
    maxelem integer,
    data text
);


ALTER TABLE public.models OWNER TO postgres;

--
-- Name: models_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE models_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.models_id_seq OWNER TO postgres;

--
-- Name: models_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE models_id_seq OWNED BY models.id;


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY models ALTER COLUMN id SET DEFAULT nextval('models_id_seq'::regclass);


--
-- Name: elements_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres; Tablespace: 
--

ALTER TABLE ONLY elements
    ADD CONSTRAINT elements_pkey PRIMARY KEY (modelid, id);


--
-- Name: models_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres; Tablespace: 
--

ALTER TABLE ONLY models
    ADD CONSTRAINT models_pkey PRIMARY KEY (id);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--


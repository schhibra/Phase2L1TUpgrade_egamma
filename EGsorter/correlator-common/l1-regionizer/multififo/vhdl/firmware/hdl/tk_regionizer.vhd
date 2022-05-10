library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.regionizer_data.all;

entity tk_regionizer is
    generic(
       USE_ALSO_COORDS_AT_VTX : boolean := true
    );
    port(
            ap_clk : IN STD_LOGIC;
            ap_rst : IN STD_LOGIC;
            ap_start : IN STD_LOGIC;
            ap_done : OUT STD_LOGIC;
            ap_idle : OUT STD_LOGIC;
            ap_ready : OUT STD_LOGIC;
            newevent : IN STD_LOGIC;
            tracks_in_0_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_0_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_1_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_1_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_2_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_2_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_3_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_3_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_4_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_4_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_5_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_5_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_6_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_6_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_7_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_7_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_8_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_8_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_0_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_1_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_2_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_3_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_4_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_5_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_6_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_7_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_8_V : OUT STD_LOGIC_VECTOR (71 downto 0);
            tracks_out_valid_0 : OUT STD_LOGIC;
            tracks_out_valid_1 : OUT STD_LOGIC;
            tracks_out_valid_2 : OUT STD_LOGIC;
            tracks_out_valid_3 : OUT STD_LOGIC;
            tracks_out_valid_4 : OUT STD_LOGIC;
            tracks_out_valid_5 : OUT STD_LOGIC;
            tracks_out_valid_6 : OUT STD_LOGIC;
            tracks_out_valid_7 : OUT STD_LOGIC;
            tracks_out_valid_8 : OUT STD_LOGIC;
            newevent_out : OUT STD_LOGIC

    );
end tk_regionizer;

architecture Behavioral of tk_regionizer is
    constant NREGIONS  : natural := NTKSECTORS;
    constant NALLFIFOS : natural := NTKSECTORS*NTKFIFOS;
    constant NMERGE2   : natural := NTKSECTORS*(NTKFIFOS/2);

    --constant DEBUG_SECTOR : natural := 0;

    --type w64_vec     is array(natural range <>) of std_logic_vector(63 downto 0);

    signal links_in :       particles(NTKSECTORS*NTKFIBERS-1 downto 0) := (others => null_particle);
    signal fifo_in :        particles(NALLFIFOS-1 downto 0);
    signal fifo_in_write :  std_logic_vector(NALLFIFOS-1 downto 0) := (others => '0');
    signal fifo_in_roll  :  std_logic_vector(NALLFIFOS-1 downto 0) := (others => '0');

    signal fifo_out :         anyparticles(NALLFIFOS-1 downto 0);
    signal fifo_out_valid :   std_logic_vector(NALLFIFOS-1 downto 0) := (others => '0');
    signal fifo_out_full:     std_logic_vector(NALLFIFOS-1 downto 0) := (others => '0');
    signal fifo_out_roll:     std_logic_vector(NALLFIFOS-1 downto 0) := (others => '0');
    --signal fifo_dbg :         w64_vec(NALLFIFOS-1 downto 0);

    signal merged2_out :        anyparticles(NMERGE2-1 downto 0);
    signal merged2_out_valid :  std_logic_vector(NMERGE2-1 downto 0) := (others => '0');
    signal merged2_out_roll:    std_logic_vector(NMERGE2-1 downto 0) := (others => '0');
    signal merged2_out_full:    std_logic_vector(NMERGE2-1 downto 0) := (others => '0');
    --signal merged2_dbg :        w64_vec(NMERGE2-1 downto 0);

    signal merged_out :        anyparticles(NREGIONS-1 downto 0);
    signal merged_out_valid :  std_logic_vector(NREGIONS-1 downto 0) := (others => '0');
    signal merged_out_roll:    std_logic_vector(NREGIONS-1 downto 0) := (others => '0');
    --signal merged_dbg :        w64_vec(NREGIONS-1 downto 0);

begin

    router : entity work.tk_router 
                generic map(USE_VTX => USE_ALSO_COORDS_AT_VTX)
                port map(ap_clk => ap_clk, 
                             enabled => ap_start,
                             newevent => newevent,
                             links_in => links_in,
                             fifo_in => fifo_in,
                             fifo_in_write => fifo_in_write,
                             fifo_in_roll  => fifo_in_roll);

    gen_fifos: for ireg in NALLFIFOS-1 downto 0 generate
        reg_buffer : entity work.rolling_fifo
                        --generic map(FIFO_INDEX => ireg+1)
                        port map(ap_clk => ap_clk, 
                                 d_in    => particle_to_any(fifo_in(ireg)),
                                 write_in  => fifo_in_write(ireg),
                                 roll   => fifo_in_roll(ireg),
                                 d_out    => fifo_out(ireg),
                                 valid_out  => fifo_out_valid(ireg),
                                 --dbg_w64 =>  fifo_dbg(ireg),
                                 full  => fifo_out_full(ireg),
                                 roll_out  => fifo_out_roll(ireg)
                             );
        end generate gen_fifos;

    gen_merger2s: for imerge in NMERGE2-1 downto 0 generate
        reg_merger2 : entity work.fifo_merge2_full
                        --generic map(FIFO_INDEX => imerge+1)
                        port map(ap_clk => ap_clk, 
                                 d1_in => fifo_out(imerge*2),
                                 d2_in => fifo_out(imerge*2+1),
                                 d1_valid => fifo_out_valid(imerge*2),
                                 d2_valid => fifo_out_valid(imerge*2+1),
                                 roll     => fifo_out_roll(imerge*2),
                                 full     => merged2_out_full(imerge),
                                 d_out      => merged2_out(imerge),
                                 valid_out  => merged2_out_valid(imerge),
                                 full1      => fifo_out_full(imerge*2),  
                                 full2      => fifo_out_full(imerge*2+1),
                                 --dbg_w64    => merged2_dbg(imerge),
                                 roll_out   => merged2_out_roll(imerge)
                            );
        end generate gen_merger2s;

    gen_merger3s: for imerge in NREGIONS-1 downto 0 generate
        reg_merger3 : entity work.fifo_merge3
                        --generic map(FIFO_INDEX => imerge+1)
                        port map(ap_clk => ap_clk, 
                                 d1_in => merged2_out(imerge*3),
                                 d2_in => merged2_out(imerge*3+1),
                                 d3_in => merged2_out(imerge*3+2),
                                 d1_valid => merged2_out_valid(imerge*3),
                                 d2_valid => merged2_out_valid(imerge*3+1),
                                 d3_valid => merged2_out_valid(imerge*3+2),
                                 roll     => merged2_out_roll(imerge*3),
                                 d_out      => merged_out(imerge),
                                 valid_out  => merged_out_valid(imerge),
                                 full1      => merged2_out_full(imerge*3),  
                                 full2      => merged2_out_full(imerge*3+1),
                                 full3      => merged2_out_full(imerge*3+2),
                                 --dbg_w64    => merged_dbg(imerge),
                                 roll_out   => merged_out_roll(imerge)
                            );
        end generate gen_merger3s;

    links_in( 0) <= w72_to_particle(tracks_in_0_0_V);
    links_in( 1) <= w72_to_particle(tracks_in_0_1_V);
    links_in( 2) <= w72_to_particle(tracks_in_1_0_V);
    links_in( 3) <= w72_to_particle(tracks_in_1_1_V);
    links_in( 4) <= w72_to_particle(tracks_in_2_0_V);
    links_in( 5) <= w72_to_particle(tracks_in_2_1_V);
    links_in( 6) <= w72_to_particle(tracks_in_3_0_V);
    links_in( 7) <= w72_to_particle(tracks_in_3_1_V);
    links_in( 8) <= w72_to_particle(tracks_in_4_0_V);
    links_in( 9) <= w72_to_particle(tracks_in_4_1_V);
    links_in(10) <= w72_to_particle(tracks_in_5_0_V);
    links_in(11) <= w72_to_particle(tracks_in_5_1_V);
    links_in(12) <= w72_to_particle(tracks_in_6_0_V);
    links_in(13) <= w72_to_particle(tracks_in_6_1_V);
    links_in(14) <= w72_to_particle(tracks_in_7_0_V);
    links_in(15) <= w72_to_particle(tracks_in_7_1_V);
    links_in(16) <= w72_to_particle(tracks_in_8_0_V);
    links_in(17) <= w72_to_particle(tracks_in_8_1_V);

    tracks_out_0_V <= anyparticle_to_w72(merged_out(0));
    tracks_out_1_V <= anyparticle_to_w72(merged_out(1));
    tracks_out_2_V <= anyparticle_to_w72(merged_out(2));
    tracks_out_3_V <= anyparticle_to_w72(merged_out(3));
    tracks_out_4_V <= anyparticle_to_w72(merged_out(4));
    tracks_out_5_V <= anyparticle_to_w72(merged_out(5));
    tracks_out_6_V <= anyparticle_to_w72(merged_out(6));
    tracks_out_7_V <= anyparticle_to_w72(merged_out(7));
    tracks_out_8_V <= anyparticle_to_w72(merged_out(8));
    tracks_out_valid_0 <= merged_out_valid(0);
    tracks_out_valid_1 <= merged_out_valid(1);
    tracks_out_valid_2 <= merged_out_valid(2);
    tracks_out_valid_3 <= merged_out_valid(3);
    tracks_out_valid_4 <= merged_out_valid(4);
    tracks_out_valid_5 <= merged_out_valid(5);
    tracks_out_valid_6 <= merged_out_valid(6);
    tracks_out_valid_7 <= merged_out_valid(7);
    tracks_out_valid_8 <= merged_out_valid(8);

    newevent_out <= merged_out_roll(0);

end Behavioral;
